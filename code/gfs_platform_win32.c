/*
 * FILE      gfs_platform_win32.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs_platform.h"

#include <Windows.h>
#include <shlwapi.h> // PathFileExistsA

#include <xinput.h>
#include <dsound.h> // TODO(ilya.a): Remove as soon as you rewrite it in XAudio2 [2024/09/01]
#include <xaudio2.h>

#include <glad/glad.h>

#include "gfs_assert.h"
#include "gfs_physics.h"
#include "gfs_types.h"
#include "gfs_memory.h"
#include "gfs_game_state.h"
#include "gfs_game.h"
#include "gfs_string.h"
#include "gfs_macros.h"
#include "gfs_render.h"

#include "gfs_render_opengl.h"

#define VCALL(S, M, ...) (S)->lpVtbl->M((S), __VA_ARGS__)
#define ASSERT_VCALL(S, M, ...)                                                \
    ASSERT(SUCCEEDED((S)->lpVtbl->M((S), __VA_ARGS__)))

// See https://gist.github.com/nickrolfe/1127313ed1dbf80254b614a721b3ee9c
// See
// https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
// for all values
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023

#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_TYPE_RGBA_ARB 0x202B

// See
// https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
// for all values
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

/// Dynamicly loaded procs

#define WIN32_GL_CREATECONTEXTATTRIBARB_PROCNAME "wglCreateContextAttribsARB"
#define WIN32_GL_CHOOSEPIXELFORMATARB_PROCNAME "wglChoosePixelFormatARB"

typedef HGLRC WINAPI
Win32_GL_CreateContextAttribARBType(HDC, HGLRC, const int *);
typedef BOOL WINAPI Win32_GL_ChoosePixelFormatARBType(
    HDC, const int *, const FLOAT *, UINT, int *, UINT *);

static Win32_GL_CreateContextAttribARBType *Win32_GL_CreateContextAttribARBPtr;
static Win32_GL_ChoosePixelFormatARBType *Win32_GL_ChoosePixelFormatARBPtr;

#define WIN32_XINPUTGETSTATE_PROCNAME "XInputGetState"
#define WIN32_XINPUTSETSTATE_PROCNAME "XInputSetState"
#define WIN32_XINPUT_DLL XINPUT_DLL

typedef DWORD Win32_XInputGetStateType(DWORD dwUserIndex, XINPUT_STATE *pState);
typedef DWORD Win32_XInputSetStateType(
    DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);

static Win32_XInputGetStateType *Win32_xInputGetStatePtr;
static Win32_XInputSetStateType *Win32_xInputSetStatePtr;

#define WIN32_DIRECTSOUNDCREATE_PROCNAME "DirectSoundCreate"
#define WIN32_DIRECTSOUND_DLL "dsound.dll"

typedef HRESULT Win32_DirectSoundCreateType(
    LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

static Win32_DirectSoundCreateType *Win32_DirectSoundCreatePtr;

static Renderer gRenderer;

typedef struct Window {
    HWND windowHandle;
    WNDCLASS windowClass;
    HDC deviceContext;
    BITMAPINFO bitMapInfo;
    HGLRC renderContext;
} Window;

typedef struct FileHandle {
    HANDLE win32Handle;
} FileHandle;

typedef struct SoundDevice {
    // Win32_SoundOutput soundOutput;
    LPDIRECTSOUNDBUFFER audioBuffer;
} SoundDevice;

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
FileHandleIsValid(FileHandle *handle) {
    return handle->win32Handle != INVALID_HANDLE_VALUE;
}

FileOpenResult
FileOpenEx(cstring8 filePath, Scratch *allocator, Permissions permissions) {
    ASSERT_NONNULL(filePath);
    ASSERT(!CString8IsEmpty(filePath));

    FileOpenResult result;
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

    HANDLE win32Handle = CreateFileA(
        filePath, desiredAccess, shareMode, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (win32Handle == INVALID_HANDLE_VALUE) {
        result.code = PLATFORM_FILE_OPEN_FAILED_TO_OPEN;
        result.handle = NULL;
    } else {
        result.handle = ScratchAlloc(allocator, sizeof(FileHandle));
        result.handle->win32Handle = win32Handle;
        result.code = PLATFORM_FILE_OPEN_OK;
    }
    return result;
}

FileCloseResultCode
FileClose(FileHandle *handle) {
    ASSERT_NONNULL(handle);
    ASSERT_ISTRUE(FileHandleIsValid(handle));

    if (CloseHandle(handle->win32Handle) == 0) {
        return FILE_CLOSE_ERR;
    }

    return FILE_CLOSE_OK;
}

FileLoadResultCode
FileLoadToBuffer(
    FileHandle *handle, void *buffer, usize numberOfBytesToLoad,
    usize *numberOfBytesLoaded) {
    return FileLoadToBufferEx(
        handle, buffer, numberOfBytesToLoad, numberOfBytesLoaded, 0);
}

FileLoadResultCode
FileLoadToBufferEx(
    FileHandle *handle, void *buffer, usize numberOfBytesToLoad,
    usize *numberOfBytesLoaded, usize loadOffset) {
    ASSERT_NONNULL(handle);
    ASSERT(FileHandleIsValid(handle));
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
    BOOL readFileResult = ReadFile(
        handle->win32Handle, buffer, numberOfBytesToLoad, &numberOfBytesRead,
        NULL);

    if (readFileResult != TRUE) {
        return PLATFORM_FILE_FAILED_TO_READ;
    }

    if (numberOfBytesLoaded != NULL) {
        *numberOfBytesLoaded = numberOfBytesRead;
    }

    return PLATFORM_FILE_LOAD_OK;
}

// XXX: BROKEN!!!!
usize
FileGetSize(FileHandle *handle) {
    DWORD highOrderSize = 0;
    DWORD lowOrderSize = GetFileSize(handle->win32Handle, &highOrderSize);
    return MAKELONG(lowOrderSize, highOrderSize);
}

void
ProcessExit(u32 code) {
    ExitProcess(code);
}

void
PutString(cstring8 s) {
    OutputDebugString(s);
}

void
ThrowDebugBreak(void) {
    DebugBreak();
}

void
PutLastError(void) {
    DWORD win32LastErrorCode = GetLastError();

    if (win32LastErrorCode != ERROR_SUCCESS) {
        char8 printBuffer[KILOBYTES(1)];
        wsprintf(
            printBuffer, "E: Win32 GetLastError()=%i\n", win32LastErrorCode);
        OutputDebugString(printBuffer);
    }

    GLenum glErrorCode = glGetError();

    if (glErrorCode != GL_NO_ERROR) {
        char8 printBuffer[KILOBYTES(1)];
        wsprintf(printBuffer, "E: OpenGL glGetError()=%i\n", glErrorCode);
        OutputDebugString(printBuffer);
    }
}

bool
IsPathExists(cstring8 path) {
    return PathFileExistsA(path);
}

RectangleI32
WindowGetRectangle(Window *window) {
    RECT win32WindowRect;
    ASSERT_NONZERO(GetClientRect(window->windowHandle, &win32WindowRect));
    RectangleI32 windowRect = Win32_ConvertRECTToRect32(win32WindowRect);
    return windowRect;
}

void
WindowUpdate(Window *window) {
    ASSERT_ISTRUE(SwapBuffers(window->deviceContext));
}

void
WindowResize(Window *window, i32 width, i32 height) {
    Renderer *renderer = &gRenderer;

    GL_CALL(glViewport(0, 0, width, height));

    UNUSED(renderer);
    UNUSED(window);
    UNUSED(width);
    UNUSED(height);
}

static void
Win32_OpenGLContextExts_Init(void) {

    WNDCLASSA dummyWindowClass = {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = DefWindowProcA,
        .hInstance = GetModuleHandle(0),
        .lpszClassName = "__dummy_window_class",
    };
    ASSERT_NONZERO(RegisterClass(&dummyWindowClass));

    HWND dummyWindowHandle = CreateWindowExA(
        0, dummyWindowClass.lpszClassName, "__dummy_window", 0, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
        dummyWindowClass.hInstance, 0);
    ASSERT_NONNULL(dummyWindowHandle);

    HDC dummyDeviceContext = GetDC(dummyWindowHandle);

    // NOTE(ilya.a): The worst struct I ever met [2024/09/07]
    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
    pixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pixelFormatDescriptor.nVersion = 1;
    pixelFormatDescriptor.dwFlags =
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
    pixelFormatDescriptor.cColorBits = 32;
    pixelFormatDescriptor.cRedBits = 0;
    pixelFormatDescriptor.cRedShift = 0;
    pixelFormatDescriptor.cGreenBits = 0;
    pixelFormatDescriptor.cGreenShift = 0;
    pixelFormatDescriptor.cBlueBits = 0;
    pixelFormatDescriptor.cBlueShift = 0;
    pixelFormatDescriptor.cAlphaBits = 8; // Should be zero?
    pixelFormatDescriptor.cAlphaShift = 0;
    pixelFormatDescriptor.cAccumBits = 0;
    pixelFormatDescriptor.cAccumRedBits = 0;
    pixelFormatDescriptor.cAccumGreenBits = 0;
    pixelFormatDescriptor.cAccumBlueBits = 0;
    pixelFormatDescriptor.cAccumAlphaBits = 0;
    pixelFormatDescriptor.cDepthBits = 24; // Number of bits for the depthbuffer
    pixelFormatDescriptor.cStencilBits =
        8; // Number of bits for the stencilbuffer
    pixelFormatDescriptor.cAuxBuffers =
        0; // Number of Aux buffers in the framebuffer.
    pixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE;
    pixelFormatDescriptor.bReserved = 0;
    pixelFormatDescriptor.dwLayerMask = 0;
    pixelFormatDescriptor.dwVisibleMask = 0;
    pixelFormatDescriptor.dwDamageMask = 0;

    int pixelFormatIndex =
        ChoosePixelFormat(dummyDeviceContext, &pixelFormatDescriptor);
    ASSERT_NONZERO(pixelFormatIndex);
    ASSERT_ISTRUE(SetPixelFormat(
        dummyDeviceContext, pixelFormatIndex, &pixelFormatDescriptor));

    HGLRC dummyRenderContext = wglCreateContext(dummyDeviceContext);
    ASSERT_NONNULL(dummyRenderContext);

    ASSERT_ISTRUE(wglMakeCurrent(dummyDeviceContext, dummyRenderContext));

    Win32_GL_CreateContextAttribARBPtr =
        (Win32_GL_CreateContextAttribARBType *)wglGetProcAddress(
            WIN32_GL_CREATECONTEXTATTRIBARB_PROCNAME);
    Win32_GL_ChoosePixelFormatARBPtr =
        (Win32_GL_ChoosePixelFormatARBType *)wglGetProcAddress(
            WIN32_GL_CHOOSEPIXELFORMATARB_PROCNAME);

    wglMakeCurrent(dummyDeviceContext, 0);
    wglDeleteContext(dummyRenderContext);
    ReleaseDC(dummyWindowHandle, dummyDeviceContext);
    DestroyWindow(dummyWindowHandle);
}

static HGLRC
Win32_OpenGLContext_Init(HDC deviceContext) {
    Win32_OpenGLContextExts_Init();

    int pixelFormatAttribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,
        GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,
        GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,
        GL_TRUE,
        WGL_ACCELERATION_ARB,
        WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,
        WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,
        32,
        WGL_DEPTH_BITS_ARB,
        24,
        WGL_STENCIL_BITS_ARB,
        8,
        0};

    int pixelFormat;
    UINT numFormats;
    ASSERT_ISTRUE(Win32_GL_ChoosePixelFormatARBPtr(
        deviceContext, pixelFormatAttribs, 0, 1, &pixelFormat, &numFormats));
    ASSERT_NONZERO(numFormats);

    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
    DescribePixelFormat(
        deviceContext, pixelFormat, sizeof(pixelFormatDescriptor),
        &pixelFormatDescriptor);
    ASSERT_ISTRUE(
        SetPixelFormat(deviceContext, pixelFormat, &pixelFormatDescriptor));

    // Specify that we want to create an OpenGL 3.3 core profile context
    int glAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB,
        3,
        WGL_CONTEXT_MINOR_VERSION_ARB,
        3,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    HGLRC renderContext =
        Win32_GL_CreateContextAttribARBPtr(deviceContext, 0, glAttribs);
    ASSERT_NONNULL(renderContext);

    ASSERT_ISTRUE(wglMakeCurrent(deviceContext, renderContext));

    return renderContext;
}

static LRESULT CALLBACK
Win32_MainWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    switch (message) {
    case WM_ACTIVATEAPP: {
    } break;
    case WM_CLOSE: {
        GameStateStop();
    } break;
    default: {
        result = DefWindowProc(window, message, wParam, lParam);
    } break;
    }
    return result;
}

int WINAPI
WinMain(
    _In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance,
    _In_ LPSTR commandLine, _In_ int showMode) {
    UNUSED(instance);
    UNUSED(commandLine);
    UNUSED(showMode);
    UNUSED(prevInstance);

    GameMainloop(&gRenderer);

    return 0;
}

typedef enum {
    WIN32_LOADXINPUT_OK,
    WIN32_LOADXINPUT_ERR
} Win32_LoadXInputResult;

static Win32_LoadXInputResult
Win32_LoadXInput(void) {
    // TODO(ilya.a): Handle different versions of xinput. Check for newer. If
    // fails, use older one. [2024/05/24]
    HMODULE library = LoadLibrary(WIN32_XINPUT_DLL);

    if (library == NULL) {
        return WIN32_LOADXINPUT_ERR;
    }

    Win32_xInputGetStatePtr = (Win32_XInputGetStateType *)GetProcAddress(
        library, WIN32_XINPUTGETSTATE_PROCNAME);

    Win32_xInputSetStatePtr = (Win32_XInputSetStateType *)GetProcAddress(
        library, WIN32_XINPUTSETSTATE_PROCNAME);

    if (Win32_xInputSetStatePtr == NULL || Win32_xInputGetStatePtr == NULL) {
        return WIN32_LOADXINPUT_ERR;
    }

    return WIN32_LOADXINPUT_OK;
}

Window *
WindowOpen(Scratch *scratch, i32 width, i32 height, cstring8 title) {
    ASSERT_NONNULL(scratch);
    ASSERT_NONNULL(title);

    usize titleLength = CString8GetLength(title);
    char8 *copiedTitle = ScratchAlloc(scratch, titleLength + 1);
    MemoryCopy(copiedTitle, title, titleLength + 1);

    Window *window = ScratchAlloc(scratch, sizeof(Window));
    ASSERT_NONNULL(window);

    MemoryZero(window, sizeof(Window));

    HINSTANCE instance = GetModuleHandle(NULL);

    window->windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    // NOTE(ilya.a): Needed by OpenGL. See Khronos's docs ^^^^^^^^

    window->windowClass.lpfnWndProc = Win32_MainWindowProc;
    window->windowClass.hInstance = instance;
    window->windowClass.lpszClassName = copiedTitle;
    ASSERT_NONZERO(RegisterClass(&window->windowClass));

    window->windowHandle = CreateWindowExA(
        0, window->windowClass.lpszClassName, copiedTitle,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
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

    window->renderContext = Win32_OpenGLContext_Init(window->deviceContext);
    ASSERT_NONNULL(window->renderContext);

    //< Glad initialization
    int version = gladLoadGL();
    ASSERT_NONZERO(version);

    GL_CALL(glViewport(0, 0, width, height));
    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    ShowWindow(window->windowHandle, SW_SHOW);
    gRenderer = RendererMake(window, COLOR_WHITE);
    WindowResize(window, width, height);

    ASSERT_EQ(Win32_LoadXInput(), WIN32_LOADXINPUT_OK);

    return window;
}

void
PoolEvents(Window *window) {
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

    for (DWORD xControllerIndex = 0; xControllerIndex < XUSER_MAX_COUNT;
         ++xControllerIndex) {
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

            // TODO(ilya.a): Handle deadzone propery, using
            // XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE and
            // XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE [2024/08/11]
            // xOffset += leftStickX / 4096;
            // yOffset += leftStickY / 4096;

            // Win32_SoundOutputSetTone(&soundOutput, (i32)(256.0f *
            // ((f32)leftStickY / 30000.0f)) + 512);

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
            // But it will be too frequent log, cause we have no plans on
            // multiple controllers.
        } else {
            OutputDebugString(
                "E: Some of xInput's device are not connected!\n");
            ASSERT(false);
        }
    }
}

void
WindowClose(Window *window) {
    RendererDestroy(&gRenderer);
    ReleaseDC(window->windowHandle, window->deviceContext);
    CloseWindow(window->windowHandle);
}

usize
GetPageSize(void) {
    usize pageSize;

    SYSTEM_INFO systemInfo = {0};
    GetSystemInfo(&systemInfo);

    pageSize = systemInfo.dwPageSize;

    return pageSize;
}

void *
MemoryAllocate(usize size) {
    void *data =
        VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    //                              ^^^^
    // NOTE(ilya.a): So, here I am reserving `size` amount of bytes, but
    // accually `VirtualAlloc` will round up this number to next page.
    // [2024/05/26]
    // TODO(ilya.a): Do something about waste of unused memory in Arena.
    // [2024/05/26]
    return data;
}

MemoryFreeResultCode
MemoryFree(void *data) {
    if (VirtualFree(data, 0, MEM_RELEASE) == 0) {
        //                   ^^^^^^^^^^^^
        // NOTE(ilya.a): Might be more reasonable to use MEM_DECOMMIT instead
        // for MEM_RELEASE. Because in that case it's will be keep buffer
        // around, until we use it again. P.S. Also will be good to try protect
        // buffer after deallocating or other stuff.
        //
        return PLATFORM_MEMORY_FREE_ERR;
    }
    return PLATFORM_MEMORY_FREE_OK;
}

typedef enum {
    WIN32_DIRECTSOUND_INIT_OK,
    WIN32_DIRECTSOUND_INIT_ERR,
    WIN32_DIRECTRSOUND_INIT_DLL_LOAD_ERR
} Win32_DirectSoundInitResult;

/*
 * Loads DirectrSound library and initializes it.
 *
 * NOTE(ilya.a): They say, that DirectSound is superseeded by WASAPI.
 * [2024/05/25]
 * TODO(ilya.a): Check this out. [2024/05/25]
 */
static Win32_DirectSoundInitResult
Win32_DirectSoundInit(
    HWND window, LPDIRECTSOUNDBUFFER *soundBuffer, i32 samplesPerSecond,
    usize bufferSize) {
    HMODULE library = LoadLibrary(WIN32_DIRECTSOUND_DLL);

    if (library == NULL) {
        return WIN32_DIRECTRSOUND_INIT_DLL_LOAD_ERR;
    }

    Win32_DirectSoundCreatePtr = (Win32_DirectSoundCreateType *)GetProcAddress(
        library, WIN32_DIRECTSOUNDCREATE_PROCNAME);

    if (Win32_DirectSoundCreatePtr == NULL) {
        return WIN32_DIRECTRSOUND_INIT_DLL_LOAD_ERR;
    }

    LPDIRECTSOUND directSound;
    if (!SUCCEEDED(Win32_DirectSoundCreatePtr(0, &directSound, NULL))) {
        return WIN32_DIRECTSOUND_INIT_ERR;
    }

    if (!SUCCEEDED(
            VCALL(directSound, SetCooperativeLevel, window, DSSCL_PRIORITY))) {
        return WIN32_DIRECTSOUND_INIT_ERR;
    }

    // NOTE(ilya.a): Primary buffer -- buffer which is only holds handle to
    // sound card. Windows has strange API. [2024/05/25]
    DSBUFFERDESC primaryBufferDesc;
    MemoryZero(
        &primaryBufferDesc,
        sizeof(primaryBufferDesc)); // TODO(ilya.a): Checkout if we really need
                                    // to zero buffer description. [2024/05/25]

    primaryBufferDesc.dwSize = sizeof(primaryBufferDesc);
    primaryBufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
    primaryBufferDesc.dwBufferBytes =
        0; // NOTE(ilya.a): Primary buffer size should be zero. [2024/05/25]
    primaryBufferDesc.lpwfxFormat = NULL; // NOTE(ilya.a): Primary buffer wfx
                                          // format should be NULL. [2024/05/25]

    LPDIRECTSOUNDBUFFER primarySoundbuffer;
    if (!SUCCEEDED(VCALL(
            directSound, CreateSoundBuffer, &primaryBufferDesc,
            &primarySoundbuffer, NULL))) {
        return WIN32_DIRECTSOUND_INIT_ERR;
    }

    WAVEFORMATEX waveFormat;
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 2;
    waveFormat.nSamplesPerSec = samplesPerSecond;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign =
        (waveFormat.nChannels * waveFormat.wBitsPerSample) / BYTE_BITS;
    waveFormat.nAvgBytesPerSec =
        waveFormat.nSamplesPerSec *
        waveFormat.nBlockAlign; // NOTE(ilya.a): Redundant. Lol.
    waveFormat.cbSize = 0;

    if (!SUCCEEDED(VCALL(primarySoundbuffer, SetFormat, &waveFormat))) {
        return WIN32_DIRECTSOUND_INIT_ERR;
    }

    // NOTE(ilya.a): Actual sound buffer in which we will write data.
    // [2024/05/25]
    DSBUFFERDESC secondaryBufferDesc;
    MemoryZero(
        &secondaryBufferDesc,
        sizeof(
            secondaryBufferDesc)); // TODO(ilya.a): Checkout if we really need
                                   // to zero buffer description. [2024/05/25]
    secondaryBufferDesc.dwSize = sizeof(secondaryBufferDesc);
    secondaryBufferDesc.dwFlags = 0;
    secondaryBufferDesc.dwBufferBytes = bufferSize;
    secondaryBufferDesc.lpwfxFormat = &waveFormat;

    if (!SUCCEEDED(VCALL(
            directSound, CreateSoundBuffer, &secondaryBufferDesc, soundBuffer,
            NULL))) {
        return WIN32_DIRECTSOUND_INIT_ERR;
    }

    return WIN32_DIRECTSOUND_INIT_OK;
}

SoundOutput
SoundOutputMake(i32 samplesPerSecond) {
    SoundOutput ret;

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

SoundDeviceGetCurrentPositionResult
SoundDeviceGetCurrentPosition(
    SoundDevice *device, u32 *playCursor, u32 *writeCursor) {
    if (!SUCCEEDED(VCALL(
            device->audioBuffer, GetCurrentPosition, (DWORD *)playCursor,
            (DWORD *)writeCursor))) {
        return PLATFORM_SOUND_DEVICE_GET_CURRENT_POSITION_ERR;
    }
    return PLATFORM_SOUND_DEVICE_GET_CURRENT_POSITION_OK;
}

void
SoundOutputSetTone(SoundOutput *output, i32 toneHZ) {
    output->toneHZ = toneHZ;
    output->wavePeriod = output->samplesPerSecond / output->toneHZ;
}

void
SoundDeviceLockBuffer(
    SoundDevice *device, u32 offset, u32 portionSizeToLock, void **region0,
    u32 *region0Size, void **region1, u32 *region1Size) {
    // TODO(ilya.a): Check why it's failed to lock buffer. Sound is nice, but
    // lock are failing [2024/07/28]
    ASSERT_VCALL(
        device->audioBuffer, Lock, offset, portionSizeToLock, region0,
        (LPDWORD)region0Size, region1, (LPDWORD)region1Size, 0);
}

void
SoundDeviceUnlockBuffer(
    SoundDevice *device, void *region0, u32 region0Size, void *region1,
    u32 region1Size) {
    ASSERT_VCALL(
        device->audioBuffer, Unlock, region0, region0Size, region1,
        region1Size);
}

void
SoundDevicePlay(SoundDevice *device) {
    ASSERT_VCALL(device->audioBuffer, Play, 0, 0, DSBPLAY_LOOPING);
}

SoundDevice *
SoundDeviceOpen(
    Scratch *scratch, Window *window, i32 samplesPerSecond,
    usize audioBufferSize) {
    ASSERT_NONNULL(scratch);
    ASSERT_NONNULL(window);
    ASSERT_NONNULL(window->windowHandle);

    SoundDevice *device = ScratchAlloc(scratch, sizeof(SoundDevice));
    ASSERT_NONNULL(device);

    Win32_DirectSoundInitResult result = Win32_DirectSoundInit(
        window->windowHandle, &device->audioBuffer, samplesPerSecond,
        audioBufferSize);
    ASSERT_ISOK(result);

    return device;
}

void
SoundDeviceClose(SoundDevice *device) {
    UNUSED(device);
}
