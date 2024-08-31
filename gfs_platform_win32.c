/*
 * FILE      gfs_platform_win32.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs_platform.h"

#include <Windows.h>
#include <shlwapi.h>  // PathFileExistsA

#include <xinput.h>
#include <dsound.h>

#include "gfs_assert.h"
#include "gfs_physics.h"
#include "gfs_types.h"
#include "gfs_memory.h"
#include "gfs_game_state.h"
#include "gfs_game.h"
#include "gfs_string.h"
#include "gfs_macros.h"
#include "gfs_render.h"

#define VCALL(S, M, ...) (S)->lpVtbl->M((S), __VA_ARGS__)
#define ASSERT_VCALL(S, M, ...) ASSERT(SUCCEEDED((S)->lpVtbl->M((S), __VA_ARGS__)))

#define WIN32_XINPUTGETSTATE_PROCNAME "XInputGetState"
#define WIN32_XINPUTSETSTATE_PROCNAME "XInputSetState"
#define WIN32_XINPUT_DLL XINPUT_DLL

typedef DWORD Win32_XInputGetStateType(DWORD dwUserIndex, XINPUT_STATE *pState);
typedef DWORD Win32_XInputSetStateType(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);

static Win32_XInputGetStateType *Win32_xInputGetStatePtr;
static Win32_XInputSetStateType *Win32_xInputSetStatePtr;

#define WIN32_DIRECTSOUNDCREATE_PROCNAME "DirectSoundCreate"
#define WIN32_DSOUND_DLL "dsound.dll"

typedef HRESULT Win32_DirectSoundCreateType(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

static Win32_DirectSoundCreateType *Win32_DirectSoundCreatePtr;

static Renderer gRenderer;

typedef struct PlatformWindow {
    HWND windowHandle;
    WNDCLASS windowClass;
    HDC deviceContext;
    BITMAPINFO bitMapInfo;
} PlatformWindow;

typedef struct PlatformFileHandle {
    HANDLE win32Handle;
} PlatformFileHandle;

typedef struct {
    u32 runningSampleIndex;
    u32 toneHZ;
    i32 samplesPerSecond;
    i32 toneVolume;
    i32 wavePeriod;
    usize bytesPerSample;
    usize audioBufferSize;

    i32 latencySampleCount;
} Win32_SoundOutput;

typedef struct PlatformSoundDevice {
    //Win32_SoundOutput soundOutput;
    LPDIRECTSOUNDBUFFER audioBuffer;
} PlatformSoundDevice;

static inline void
Win32_GetRectSize(const RECT *r, i32 *width, i32 *height) {
    *width = r->right - r->left;
    *height = r->bottom - r->top;
}

static inline RectangleI32
Win32_ConvertRECTToRect32(RECT rect) {
    RectangleI32 result;
    result.x = rect.left;
    result.y = rect.top;
    Win32_GetRectSize(&rect, &result.width, &result.height);
    return result;
}

bool
PlatformFileHandleIsValid(PlatformFileHandle *handle) {
    return handle->win32Handle != INVALID_HANDLE_VALUE;
}

PlatformFileOpenResult
PlatformFileOpenEx(cstring8 filePath, ScratchAllocator *allocator, PlatformPermissions permissions) {
    ASSERT_NONNULL(filePath);
    ASSERT(!CString8IsEmpty(filePath));

    PlatformFileOpenResult result;
    DWORD desiredAccess = 0;

    if (HASANYBIT(permissions, PLATFORM_PERMISSION_READ)) {
        desiredAccess |= GENERIC_READ;
    }

    if (HASANYBIT(permissions, PLATFORM_PERMISSION_WRITE)) {
        desiredAccess |= GENERIC_WRITE;
    }

    // TODO(ilya.a): Expose sharing options [2024/05/26]
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
    DWORD shareMode = FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE;

    HANDLE win32Handle = CreateFileA(filePath, desiredAccess, shareMode, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (win32Handle == INVALID_HANDLE_VALUE) {
        result.code = PLATFORM_FILE_OPEN_FAILED_TO_OPEN;
        result.handle = NULL;
    } else {
        result.handle = ScratchAllocatorAlloc(allocator, sizeof(PlatformFileHandle));
        result.handle->win32Handle = win32Handle;
        result.code = PLATFORM_FILE_OPEN_OK;
    }
    return result;
}

PlatformFileLoadResult
PlatformFileLoadToBuffer(PlatformFileHandle *handle, void *buffer, usize numberOfBytesToLoad,  usize *numberOfBytesLoaded) {
    return PlatformFileLoadToBufferEx(handle, buffer, numberOfBytesToLoad, numberOfBytesLoaded, 0);
}

PlatformFileLoadResult
PlatformFileLoadToBufferEx(PlatformFileHandle *handle, void *buffer, usize numberOfBytesToLoad, usize *numberOfBytesLoaded, usize loadOffset) {
    ASSERT_NONNULL(handle);
    ASSERT(PlatformFileHandleIsValid(handle));
    ASSERT_NONNULL(buffer);
    ASSERT_NONZERO(numberOfBytesToLoad);


#if 0
    // TODO(ilya.a): Check what was wrong about this piece of code.
    // (Unable to offset and read properly). [2024/05/26]

    LARGE_INTEGER offsetPair;
    offsetPair.LowPart = ((U32 *)&offset)[1];
    offsetPair.HighPart = ((U32 *)&offset)[0];
#endif

    DWORD setFilePointerResult = SetFilePointer(
        handle->win32Handle,
        (u32)loadOffset, // XXX(ilya.a): Hack [2024/05/26]
        NULL, FILE_BEGIN);

    if (setFilePointerResult == INVALID_SET_FILE_POINTER) {
        return PLATFORM_FILE_FAILED_TO_READ;
    }

    DWORD numberOfBytesRead = 0;
    BOOL readFileResult = ReadFile(handle->win32Handle, buffer, numberOfBytesToLoad, &numberOfBytesRead, NULL);

    if (readFileResult != TRUE) {
        return PLATFORM_FILE_FAILED_TO_READ;
    }

    if (numberOfBytesLoaded != NULL) {
        *numberOfBytesLoaded = numberOfBytesRead;
    }

    return PLATFORM_FILE_LOAD_OK;
}


void
PlatformExitProcess(u32 code) {
    ExitProcess(code);
}

void
PlatformPutString(cstring8 s) {
    OutputDebugString(s);
}

void
PlatformDebugBreak(void) {
    DebugBreak();
}

bool
PlatformIsPathExists(cstring8 path) {
    return PathFileExistsA(path);
}

RectangleI32
PlatformWindowGetRectangle(PlatformWindow *window) {
    RECT win32WindowRect;
    ASSERT_NONZERO(GetClientRect(window->windowHandle, &win32WindowRect));
    RectangleI32 windowRect = Win32_ConvertRECTToRect32(win32WindowRect);
    return windowRect;
}

void
PlatformWindowUpdate(PlatformWindow *window, i32 windowXOffset, i32 windowYOffset, i32 windowWidth, i32 windowHeight) {
    StretchDIBits(
        window->deviceContext, windowXOffset, windowYOffset, windowWidth, windowHeight, gRenderer.xOffset, gRenderer.yOffset,
        gRenderer.pixels.Width, gRenderer.pixels.Height, gRenderer.pixels.Buffer, &window->bitMapInfo, DIB_RGB_COLORS,
        SRCCOPY);
}

void
PlatformWindowResize(PlatformWindow *window, i32 width, i32 height) {
    Renderer *renderer = &gRenderer;

    // TODO(ilya.a):
    //     - [ ] Checkout how it works.
    //     - [ ] Handle allocation error.
    ASSERT(!(renderer->pixels.Buffer != NULL && PlatformMemoryFree(renderer->pixels.Buffer) != PLATFORM_MEMORY_FREE_ERR));

    renderer->pixels.Width = width;
    renderer->pixels.Height = height;

    window->bitMapInfo.bmiHeader.biSize = sizeof(window->bitMapInfo.bmiHeader);
    window->bitMapInfo.bmiHeader.biWidth = width;
    window->bitMapInfo.bmiHeader.biHeight = height; // NOTE: Treat coordinates bottom-up. Can flip sign and make it top-down.
    window->bitMapInfo.bmiHeader.biPlanes = 1;
    window->bitMapInfo.bmiHeader.biBitCount = 32; // NOTE: Align to WORD
    window->bitMapInfo.bmiHeader.biCompression = BI_RGB;
    window->bitMapInfo.bmiHeader.biSizeImage = 0;
    window->bitMapInfo.bmiHeader.biXPelsPerMeter = 0;
    window->bitMapInfo.bmiHeader.biYPelsPerMeter = 0;
    window->bitMapInfo.bmiHeader.biClrUsed = 0;
    window->bitMapInfo.bmiHeader.biClrImportant = 0;


    usize bufferSize = width * height * renderer->bytesPerPixel;
    renderer->pixels.Buffer = PlatformMemoryAllocate(bufferSize);
    ASSERT_NONNULL(renderer->pixels.Buffer);
}

static LRESULT CALLBACK
Win32_MainWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    switch (message) {
    case WM_ACTIVATEAPP: {
        OutputDebugString("T: WM_ACTIVATEAPP\n");
    } break;
    case WM_CLOSE: {
        // TODO(ilya.a): Ask for closing?
        OutputDebugString("T: WM_CLOSE\n");
        GameStateStop();
    } break;
    case WM_DESTROY: {
        // TODO(ilya.a): Casey says that we maybe should recreate window later?
        OutputDebugString("T: WM_DESTROY\n");
        // PostQuitMessage(0);
    } break;
    default: {
        // NOTE(ilya): Leave other events to default Window's handler.
        result = DefWindowProc(window, message, wParam, lParam);
    } break;
    }

    return result;
}

int WINAPI
WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance, _In_ LPSTR commandLine, _In_ int showMode) {
    UNUSED(instance);
    UNUSED(commandLine);
    UNUSED(showMode);
    UNUSED(prevInstance);

    GameMainloop(&gRenderer);
}

typedef enum { WIN32_LOADXINPUT_OK, WIN32_LOADXINPUT_ERR } Win32_LoadXInputResult;

static Win32_LoadXInputResult
Win32_LoadXInput(void) {
    // TODO(ilya.a): Handle different versions of xinput. Check for newer. If
    // fails, use older one. [2024/05/24]
    HMODULE library = LoadLibrary(WIN32_XINPUT_DLL);

    if (library == NULL) {
        return WIN32_LOADXINPUT_ERR;
    }

    Win32_xInputGetStatePtr = (Win32_XInputGetStateType *)GetProcAddress(library, WIN32_XINPUTGETSTATE_PROCNAME);

    Win32_xInputSetStatePtr = (Win32_XInputSetStateType *)GetProcAddress(library, WIN32_XINPUTSETSTATE_PROCNAME);

    if (Win32_xInputSetStatePtr == NULL || Win32_xInputGetStatePtr == NULL) {
        return WIN32_LOADXINPUT_ERR;
    }

    return WIN32_LOADXINPUT_OK;
}

PlatformWindow *
PlatformWindowOpen(ScratchAllocator *scratch, i32 width, i32 height, cstring8 title) {
    ASSERT_NONNULL(scratch);
    ASSERT_NONNULL(title);

    usize titleLength = CString8GetLength(title);
    char8 *copiedTitle = ScratchAllocatorAlloc(scratch, titleLength + 1);
    MemoryCopy(copiedTitle, title, titleLength + 1);

    PlatformWindow *window = ScratchAllocatorAlloc(scratch, sizeof(PlatformWindow));
    ASSERT_NONNULL(window);

    MemoryZero(window, sizeof(PlatformWindow));

    HINSTANCE instance = GetModuleHandle(NULL);

    window->windowClass.style = CS_VREDRAW | CS_HREDRAW;
    window->windowClass.lpfnWndProc = Win32_MainWindowProc;
    window->windowClass.hInstance = instance;
    window->windowClass.lpszClassName = copiedTitle;
    ASSERT_NONZERO(RegisterClass(&window->windowClass));

    window->windowHandle = CreateWindowExA(
        0, window->windowClass.lpszClassName, copiedTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, // int x
        CW_USEDEFAULT, // int y
        width,         // int width
        height,        // int height
        NULL,          // windowParent
        NULL,          // menu
        instance, NULL);
    ASSERT_NONNULL(window->windowHandle);

    window->deviceContext = GetDC(window->windowHandle);
    ASSERT_NONNULL(window->deviceContext);

    ShowWindow(window->windowHandle, SW_SHOW);

    gRenderer = RendererMake(window, COLOR_WHITE);
    PlatformWindowResize(window, width, height);

    ASSERT_EQ(Win32_LoadXInput(), WIN32_LOADXINPUT_OK);

    return window;
}

void
PlatformPoolEvents(PlatformWindow *window) {
    UNUSED(window);

    MSG message = {};
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) {
            // NOTE(ilya.a): Make sure that we will quit the mainloop.
            GameStateStop();
        }

        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    for (DWORD xControllerIndex = 0; xControllerIndex < XUSER_MAX_COUNT; ++xControllerIndex) {
        XINPUT_STATE xInputState = {0};
        DWORD result = Win32_xInputGetStatePtr(xControllerIndex, &xInputState);

        if (result == ERROR_SUCCESS) {
            XINPUT_GAMEPAD *pad = &xInputState.Gamepad;

            bool aButton = pad->wButtons & XINPUT_GAMEPAD_A;
            bool bButton = pad->wButtons & XINPUT_GAMEPAD_B;
            bool xButton = pad->wButtons & XINPUT_GAMEPAD_X;
            bool yButton = pad->wButtons & XINPUT_GAMEPAD_Y;

            bool dPadUp = pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
            bool dPadDown = pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
            bool dPadLeft = pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
            bool dPadRight = pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

            i16 leftStickX = pad->sThumbLX;
            i16 leftStickY = pad->sThumbLY;
            i16 rightStickX = pad->sThumbRX;
            i16 rightStickY = pad->sThumbRY;

            // TODO(ilya.a): Handle deadzone propery, using XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE and
            // XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE [2024/08/11]
            // xOffset += leftStickX / 4096;
            // yOffset += leftStickY / 4096;

            // Win32_SoundOutputSetTone(&soundOutput, (i32)(256.0f * ((f32)leftStickY / 30000.0f)) + 512);

            XINPUT_VIBRATION vibrationState = {0};

            if (dPadRight || dPadLeft || dPadUp || dPadDown) {
#if 0
                    vibrationState.wLeftMotorSpeed = 60000;
                    vibrationState.wRightMotorSpeed = 60000;
#endif
            } else {
                vibrationState.wLeftMotorSpeed = 0;
                vibrationState.wRightMotorSpeed = 0;
            }

            Win32_xInputSetStatePtr(xControllerIndex, &vibrationState);
        } else if (result == ERROR_DEVICE_NOT_CONNECTED) {
            // NOTE(ilya.a): Do nothing. I thought to log that.
            // But it will be too frequent log, cause we have no plans on multiple controllers.
        } else {
            OutputDebugString("E: Some of xInput's device are not connected!\n");
            ASSERT(false);
        }
    }
}

void
PlatformWindowClose(PlatformWindow *window) {
    RendererDestroy(&gRenderer);
    ReleaseDC(window->windowHandle, window->deviceContext);
    CloseWindow(window->windowHandle);
}

usize
PlatformGetPageSize(void) {
    usize pageSize;

    SYSTEM_INFO systemInfo = {0};
    GetSystemInfo(&systemInfo);

    pageSize = systemInfo.dwPageSize;

    return pageSize;
}

void *
PlatformMemoryAllocate(usize size) {
    void *data = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    //                              ^^^^
    // NOTE(ilya.a): So, here I am reserving `size` amount of bytes, but accually `VirtualAlloc`
    // will round up this number to next page. [2024/05/26]
    // TODO(ilya.a): Do something about waste of unused memory in Arena. [2024/05/26]
    return data;
}


PlatformMemoryFreeResult
PlatformMemoryFree(void *data) {
    if (VirtualFree(data, 0, MEM_RELEASE) == 0) {
        //                   ^^^^^^^^^^^^
        // NOTE(ilya.a): Might be more reasonable to use MEM_DECOMMIT instead for
        // MEM_RELEASE. Because in that case it's will be keep buffer around, until
        // we use it again.
        // P.S. Also will be good to try protect buffer after deallocating or other
        // stuff.
        //
        return PLATFORM_MEMORY_FREE_ERR;
    }
    return PLATFORM_MEMORY_FREE_OK;
}

typedef enum { WIN32_INITDSOUND_OK, WIN32_INITDSOUND_ERR, WIN32_INITDSOUND_DLL_LOAD } Win32_InitDSoundResult;

/*
 * Loads DirectrSound library and initializes it.
 *
 * NOTE(ilya.a): They say, that DirectSound is superseeded by WASAPI. [2024/05/25]
 * TODO(ilya.a): Check this out. [2024/05/25]
 */
static Win32_InitDSoundResult
Win32_InitDSound(HWND window, LPDIRECTSOUNDBUFFER *soundBuffer, i32 samplesPerSecond, usize bufferSize) {
    HMODULE library = LoadLibrary(WIN32_DSOUND_DLL);

    if (library == NULL) {
        return WIN32_INITDSOUND_DLL_LOAD;
    }

    Win32_DirectSoundCreatePtr =
        (Win32_DirectSoundCreateType *)GetProcAddress(library, WIN32_DIRECTSOUNDCREATE_PROCNAME);

    if (Win32_DirectSoundCreatePtr == NULL) {
        return WIN32_INITDSOUND_DLL_LOAD;
    }

    LPDIRECTSOUND directSound;
    if (!SUCCEEDED(Win32_DirectSoundCreatePtr(0, &directSound, NULL))) {
        return WIN32_INITDSOUND_ERR;
    }

    if (!SUCCEEDED(VCALL(directSound, SetCooperativeLevel, window, DSSCL_PRIORITY))) {
        return WIN32_INITDSOUND_ERR;
    }

    // NOTE(ilya.a): Primary buffer -- buffer which is only holds handle to
    // sound card. Windows has strange API. [2024/05/25]
    DSBUFFERDESC primaryBufferDesc;
    MemoryZero(&primaryBufferDesc, sizeof(primaryBufferDesc)); // TODO(ilya.a): Checkout if we really need
                                                               // to zero buffer description. [2024/05/25]

    primaryBufferDesc.dwSize = sizeof(primaryBufferDesc);
    primaryBufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
    primaryBufferDesc.dwBufferBytes = 0;  // NOTE(ilya.a): Primary buffer size should be zero. [2024/05/25]
    primaryBufferDesc.lpwfxFormat = NULL; // NOTE(ilya.a): Primary buffer wfx format should be NULL. [2024/05/25]

    LPDIRECTSOUNDBUFFER primaryBuffer;
    if (!SUCCEEDED(VCALL(directSound, CreateSoundBuffer, &primaryBufferDesc, &primaryBuffer, NULL))) {
        return WIN32_INITDSOUND_ERR;
    }

    WAVEFORMATEX waveFormat;
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 2;
    waveFormat.nSamplesPerSec = samplesPerSecond;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / BYTE_BITS;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign; // NOTE(ilya.a): Redundant. Lol.
    waveFormat.cbSize = 0;

    if (!SUCCEEDED(VCALL(primaryBuffer, SetFormat, &waveFormat))) {
        return WIN32_INITDSOUND_ERR;
    }

    // NOTE(ilya.a): Actual sound buffer in which we will write data. [2024/05/25]
    DSBUFFERDESC secondaryBufferDesc;
    MemoryZero(&secondaryBufferDesc, sizeof(secondaryBufferDesc)); // TODO(ilya.a): Checkout if we really need
                                                                   // to zero buffer description. [2024/05/25]
    secondaryBufferDesc.dwSize = sizeof(secondaryBufferDesc);
    secondaryBufferDesc.dwFlags = 0;
    secondaryBufferDesc.dwBufferBytes = bufferSize;
    secondaryBufferDesc.lpwfxFormat = &waveFormat;

    if (!SUCCEEDED(VCALL(directSound, CreateSoundBuffer, &secondaryBufferDesc, soundBuffer, NULL))) {
        return WIN32_INITDSOUND_ERR;
    }

    return WIN32_INITDSOUND_OK;
}

PlatformSoundOutput
PlatformSoundOutputMake(i32 samplesPerSecond) {
    PlatformSoundOutput ret;

    ret.samplesPerSecond = samplesPerSecond;
    ret.runningSampleIndex = 0;
    ret.toneHZ = 256;
    ret.toneVolume = 1000;

    ret.wavePeriod = ret.samplesPerSecond / ret.toneHZ;
    ret.bytesPerSample = sizeof(i16) * 2;
    ret.audioBufferSize = ret.samplesPerSecond * ret.bytesPerSample;

    ret.latencySampleCount = ret.samplesPerSecond / 15;
    return ret;
}

#if 0
static void
Win32_SoundOutputSetTone(Win32_SoundOutput *soundOutput, i32 toneHZ) {
    soundOutput->toneHZ = toneHZ;
    soundOutput->wavePeriod = soundOutput->samplesPerSecond / soundOutput->toneHZ;
}

static void
Win32_FillSoundBuffer(Win32_SoundOutput *soundOutput, LPDIRECTSOUNDBUFFER soundBuffer, DWORD byteToLock, DWORD bytesToWrite) {
    VOID *region1, *region2;
    DWORD region1Size, region2Size;

    // TODO(ilya.a): Check why it's failed to lock buffer. Sound is nice, but lock are failing [2024/07/28]
    VCALL(soundBuffer, Lock, byteToLock, bytesToWrite, &region1, &region1Size, &region2, &region2Size, 0);

    DWORD region1SampleCount = region1Size / soundOutput->bytesPerSample;
    i16 *sampleOut = (i16 *)region1;
    for (u32 sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex) {
        f32 sinePosition = 2.0f * PI32 * (f32)soundOutput->runningSampleIndex / (f32)soundOutput->wavePeriod;
        f32 sineValue = sinf(sinePosition);
        i16 sampleValue = (i16)(sineValue * soundOutput->toneVolume);
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
        ++soundOutput->runningSampleIndex;
    }

    DWORD region2SampleCount = region2Size / soundOutput->bytesPerSample;
    sampleOut = (i16 *)region2;
    for (u32 sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex) {
        f32 sinePosition = 2.0f * PI32 * (f32)soundOutput->runningSampleIndex / (f32)soundOutput->wavePeriod;
        f32 sineValue = sinf(sinePosition);
        i16 sampleValue = (i16)(sineValue * soundOutput->toneVolume);
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
        ++soundOutput->runningSampleIndex;
    }

    // TODO(ilya.a): Check for succeed. [2024/05/25]
    ASSERT_VCALL(soundBuffer, Unlock, region1, region1Size, region2, region2Size);
}
#endif

void
PlatformSoundDeviceLockBuffer(PlatformSoundDevice *device, u32 offset, u32 portionSizeToLock, void **region0, u32 *region0Size, void **region1, u32 *region1Size) {
    // TODO(ilya.a): Check why it's failed to lock buffer. Sound is nice, but lock are failing [2024/07/28]
    VCALL(device->audioBuffer, Lock, offset, portionSizeToLock, region0, (LPDWORD)region0Size, region1, (LPDWORD)region1Size, 0);
}

void
PlatformSoundDeviceUnlockBuffer(PlatformSoundDevice *device, void *region0, u32 region0Size, void *region1, u32 region1Size) {
    ASSERT_VCALL(device->audioBuffer, Unlock, region0, region0Size, region1, region1Size);
}

void
PlatformSoundDevicePlay(PlatformSoundDevice *device) {
    ASSERT_VCALL(device->audioBuffer, Play, 0, 0, DSBPLAY_LOOPING);
}

PlatformSoundDevice *
PlatformSoundDeviceOpen(ScratchAllocator *scratch, PlatformWindow *window, i32 samplesPerSecond, usize audioBufferSize) {
    ASSERT_NONNULL(scratch);
    ASSERT_NONNULL(window);
    ASSERT_NONNULL(window->windowHandle);

    PlatformSoundDevice *device = ScratchAllocatorAlloc(scratch, sizeof(PlatformSoundDevice));
    ASSERT_NONNULL(device);

    // device->soundOutput = Win32_SoundOutputMake();

    Win32_InitDSoundResult initDSoundResult = Win32_InitDSound(
        window->windowHandle, &device->audioBuffer, samplesPerSecond,
        audioBufferSize);
    ASSERT_ISOK(initDSoundResult);

    // Win32_FillSoundBuffer(
    //     &device->soundOutput, device->audioBuffer, 0, device->soundOutput.latencySampleCount * device->soundOutput.bytesPerSample /* soundOutput.audioBufferSize */);

    return device;
}

void
PlatformSoundDeviceClose(PlatformSoundDevice *device) {
    UNUSED(device);
}
