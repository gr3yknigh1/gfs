/*
 * GFS. Entry point.
 *
 * FILE      gfs_main.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 *
 * This is small educational project where I
 * doodling around with Windows API and computer
 * graphics.
 *
 * Copied most of source, related to BMR from
 * my `gr3yknigh1/libsbmr` repo.
 * */

#include <Windows.h>
#include <xinput.h>
#include <dsound.h>

#include <math.h> // TODO(ilya.a): Replace with custom code [2024/06/08]

#include "gfs_types.h"
#include "gfs_macros.h"
#include "gfs_string.h"
#include "gfs_linalg.h"
#include "gfs_fs.h"
#include "gfs_wave.h"
#include "gfs_color.h"
#include "gfs_memory.h"
#include "gfs_geometry.h"
#include "gfs_win32_bmr.h"
#include "gfs_win32_keys.h"
#include "gfs_win32_misc.h"
#include "gfs_assert.h"

#define VCALL(S, M, ...) (S)->lpVtbl->M((S), __VA_ARGS__)
#define PI32 3.14159265358979323846f

#define ASSERT_VCALL(S, M, ...) GFS_ASSERT(SUCCEEDED((S)->lpVtbl->M((S), __VA_ARGS__)))

typedef DWORD Win32_XInputGetStateType(DWORD dwUserIndex, XINPUT_STATE *pState);
typedef DWORD Win32_XInputSetStateType(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);

typedef HRESULT Win32_DirectSoundCreateType(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

static Win32_XInputGetStateType *Win32_xInputGetStatePtr;
static Win32_XInputSetStateType *Win32_xInputSetStatePtr;

static Win32_DirectSoundCreateType *Win32_DirectSoundCreatePtr;

static LPDIRECTSOUNDBUFFER g_Win32_AudioBuffer;

static BMR_Renderer gRenderer;
static bool gShouldStop = false;

#define WIN32_XINPUTGETSTATE_PROCNAME "XInputGetState"
#define WIN32_XINPUTSETSTATE_PROCNAME "XInputSetState"

#define WIN32_XINPUT_DLL XINPUT_DLL

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

#define WIN32_DSOUND_DLL "dsound.dll"
#define WIN32_DIRECTSOUNDCREATE_PROCNAME "DirectSoundCreate"

typedef enum { WIN32_INITDSOUND_OK, WIN32_INITDSOUND_ERR, WIN32_INITDSOUND_DLL_LOAD } Win32_InitDSoundResult;

/*
 * Loads DirectrSound library and initializes it.
 *
 * NOTE(ilya.a): They say, that DirectSound is superseeded by WASAPI. [2024/05/25]
 * TODO(ilya.a): Check this out. [2024/05/25]
 */
static Win32_InitDSoundResult
Win32_InitDSound(HWND window, i32 samplesPerSecond, usize bufferSize) {
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

    if (!SUCCEEDED(VCALL(directSound, CreateSoundBuffer, &secondaryBufferDesc, &g_Win32_AudioBuffer, NULL))) {
        return WIN32_INITDSOUND_ERR;
    }

    return WIN32_INITDSOUND_OK;
}

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

static Win32_SoundOutput
Win32_SoundOutputMake(void) {
    Win32_SoundOutput ret = {0};
    ret.samplesPerSecond = 48000;
    ret.runningSampleIndex = 0;
    ret.toneHZ = 256;
    ret.toneVolume = 1000;

    ret.wavePeriod = ret.samplesPerSecond / ret.toneHZ;
    ret.bytesPerSample = sizeof(i16) * 2;
    ret.audioBufferSize = ret.samplesPerSecond * ret.bytesPerSample;

    ret.latencySampleCount = ret.samplesPerSecond / 15;
    return ret;
}

static void
Win32_SoundOutputSetTone(Win32_SoundOutput *soundOutput, i32 toneHZ) {
    soundOutput->toneHZ = toneHZ;
    soundOutput->wavePeriod = soundOutput->samplesPerSecond / soundOutput->toneHZ;
}

static void
Win32_FillSoundBuffer(Win32_SoundOutput *soundOutput, DWORD byteToLock, DWORD bytesToWrite) {
    VOID *region1, *region2;
    DWORD region1Size, region2Size;

    // TODO(ilya.a): Check why it's failed to lock buffer. Sound is nice, but lock are failing [2024/07/28]
    VCALL(g_Win32_AudioBuffer, Lock, byteToLock, bytesToWrite, &region1, &region1Size, &region2, &region2Size, 0);

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
    ASSERT_VCALL(g_Win32_AudioBuffer, Unlock, region1, region1Size, region2, region2Size);
}

LRESULT CALLBACK
Win32_MainWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    switch (message) {
    case WM_ACTIVATEAPP: {
        OutputDebugString("T: WM_ACTIVATEAPP\n");
    } break;
#if 0
        case WM_KEYUP:
        case WM_KEYDOWN:
        {
            // NOTE(ilya.a): Messed up input because of gamepad input. It's overrides state changed via keyboard
            // input [2024/05/24]
            U32 vkCode = wParam;
            bool wasDown = (lParam & (1 << 30)) != 0;
            bool isDown = (lParam & (1 << 31)) == 0;

            if (vkCode == VK_LEFT)
            {
                player.Input.LeftPressed = isDown;
            }
            else if (vkCode == VK_RIGHT)
            {
                player.Input.RightPressed = isDown;
            }
            else if (vkCode == VK_DOWN)
            {
                player.Input.DownPressed = isDown;
            }
            else if (vkCode == VK_UP)
            {
                player.Input.UpPressed = isDown;
            }
        } break;
#endif // #if 0
    case WM_CLOSE: {
        // TODO(ilya.a): Ask for closing?
        OutputDebugString("T: WM_CLOSE\n");
        gShouldStop = true;
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
    UNUSED(commandLine);
    UNUSED(showMode);
    UNUSED(prevInstance);

    switch (Win32_LoadXInput()) {
    case (WIN32_LOADXINPUT_OK): {
    } break;
    case (WIN32_LOADXINPUT_ERR): {
        OutputDebugString("E: Failed to load XInput functions!\n");
        return 0;
    } break;
    default: {
    } break;
    }

    static LPCSTR CLASS_NAME = "GFS";
    static LPCSTR WINDOW_TITLE = "GFS";

    WNDCLASS windowClass = {0};
    windowClass.style = CS_VREDRAW | CS_HREDRAW;
    windowClass.lpfnWndProc = Win32_MainWindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = CLASS_NAME;
    ASSERT_NONZERO(RegisterClass(&windowClass));

    HWND window = CreateWindowExA(
        0, windowClass.lpszClassName, WINDOW_TITLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, // int x
        CW_USEDEFAULT, // int y
        CW_USEDEFAULT, // int width
        CW_USEDEFAULT, // int height
        NULL,          // windowParent
        NULL,          // menu
        instance, NULL);
    ASSERT_NONNULL(window);

    ShowWindow(window, showMode);

    gRenderer = BMR_Init(COLOR_WHITE, window);
    BMR_Resize(&gRenderer, 900, 600);

    Win32_SoundOutput soundOutput = Win32_SoundOutputMake();
    Win32_InitDSoundResult initDSoundResult =
        Win32_InitDSound(window, soundOutput.samplesPerSecond, soundOutput.audioBufferSize);
    ASSERT_EQ(initDSoundResult, WIN32_INITDSOUND_OK);

    Win32_FillSoundBuffer(
        &soundOutput, 0, soundOutput.latencySampleCount * soundOutput.bytesPerSample /* soundOutput.audioBufferSize */);
    ASSERT_VCALL(g_Win32_AudioBuffer, Play, 0, 0, DSBPLAY_LOOPING);

    u32 xOffset = 0;
    u32 yOffset = 0;

    gRenderer.ClearColor = COLOR_WHITE;

    LARGE_INTEGER performanceCounterFrequency = {0};
    ASSERT_NONZERO(QueryPerformanceFrequency(&performanceCounterFrequency));

    LARGE_INTEGER lastCounter = {0};
    ASSERT_NONZERO(QueryPerformanceCounter(&lastCounter));

    u64 lastCycleCount = __rdtsc();

    /// BEGIN(MAINLOOP)
    while (!gShouldStop) {
        MSG message = {};
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                // NOTE(ilya.a): Make sure that we will quit the mainloop.
                gShouldStop = true;
            }

            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        // TODO(ilya.a): Should we pool more frequently? [2024/05/19]
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
                xOffset += leftStickX / 4096;
                yOffset += leftStickY / 4096;

                Win32_SoundOutputSetTone(&soundOutput, (i32)(256.0f * ((f32)leftStickY / 30000.0f)) + 512);

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
                return 0;
            }
        }

        BMR_BeginDrawing(&gRenderer);

        BMR_Clear(&gRenderer);
        BMR_DrawGrad(&gRenderer, xOffset, yOffset);
        BMR_DrawLine(&gRenderer, 100, 200, 500, 600);

        DWORD playCursor;
        DWORD writeCursor;

        if (SUCCEEDED(VCALL(g_Win32_AudioBuffer, GetCurrentPosition, &playCursor, &writeCursor))) {
            DWORD byteToLock =
                (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) % soundOutput.audioBufferSize;
            DWORD targetCursor = (playCursor + (soundOutput.latencySampleCount * soundOutput.bytesPerSample)) %
                                 soundOutput.audioBufferSize;
            DWORD bytesToWrite = 0;

            if (byteToLock > targetCursor) {
                bytesToWrite = soundOutput.audioBufferSize - byteToLock;
                bytesToWrite += targetCursor;
            } else {
                bytesToWrite = targetCursor - byteToLock;
            }

            Win32_FillSoundBuffer(&soundOutput, byteToLock, bytesToWrite);
        }

        BMR_EndDrawing(&gRenderer);

        xOffset++;
        yOffset++;

        {
            u64 endCycleCount = __rdtsc();

            LARGE_INTEGER endCounter;
            ASSERT_NONZERO(QueryPerformanceCounter(&endCounter));

            // TODO(ilya.a): Display counter [2024/11/08]

            u64 cyclesElapsed = endCycleCount - lastCycleCount;
            LONGLONG counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
            u64 msPerFrame = (1000 * counterElapsed) / performanceCounterFrequency.QuadPart;
            u64 framesPerSeconds = performanceCounterFrequency.QuadPart / counterElapsed;
            u64 megaCyclesPerFrame = cyclesElapsed / (1000 * 1000);

            char8 printBuffer[KILOBYTES(1)];
            wsprintf(printBuffer, "%ums/f | %uf/s | %umc/f\n", msPerFrame, framesPerSeconds, megaCyclesPerFrame);
            OutputDebugString(printBuffer);
            lastCounter = endCounter;
            lastCycleCount = endCycleCount;
        }
    }

    /// END(MAINLOOP)

    BMR_DeInit(&gRenderer);

    return 0;
}
