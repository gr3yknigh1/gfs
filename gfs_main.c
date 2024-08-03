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

#include <math.h>  // TODO(ilya.a): Replace with custom code [2024/06/08]

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
#define ASSERT_ISZERO(EXPR)     GFS_ASSERT((EXPR) == 0)

typedef DWORD Win32_XInputGetStateType(DWORD dwUserIndex, XINPUT_STATE* pState);
typedef DWORD Win32_XInputSetStateType(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

typedef HRESULT Win32_DirectSoundCreateType(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

global_var Win32_XInputGetStateType *G_Win32_XInputGetStatePtr;
global_var Win32_XInputSetStateType *G_Win32_XInputSetStatePtr;

global_var Win32_DirectSoundCreateType *G_Win32_DirectSoundCreatePtr;

global_var LPDIRECTSOUNDBUFFER G_Win32_AudioBuffer;

global_var BMR_Renderer G_Renderer;
global_var Bool G_ShouldStop = false;
global_var Bool G_IsSoundPlaying = false;

global_var struct
{
    Rect Rect;
    Color4 Color;

    struct
    {
        Bool LeftPressed;
        Bool RightPressed;
        Bool UpPressed;
        Bool DownPressed;
    } Input;
} G_Player;


#define PLAYER_INIT_X 100
#define PLAYER_INIT_Y 60
#define PLAYER_WIDTH  160
#define PLAYER_HEIGHT 80
#define PLAYER_SPEED  10


#define WIN32_XINPUTGETSTATE_PROCNAME "XInputGetState"
#define WIN32_XINPUTSETSTATE_PROCNAME "XInputSetState"

#define WIN32_XINPUT_DLL XINPUT_DLL

typedef enum
{
    WIN32_LOADXINPUT_OK,
    WIN32_LOADXINPUT_ERR
} Win32_LoadXInputResult;

internal Win32_LoadXInputResult
Win32_LoadXInput(void)
{
    // TODO(ilya.a): Handle different versions of xinput. Check for newer. If
    // fails, use older one. [2024/05/24]
    HMODULE library = LoadLibrary(WIN32_XINPUT_DLL);

    if (library == NULL)
    {
        return WIN32_LOADXINPUT_ERR;
    }

    G_Win32_XInputGetStatePtr = (Win32_XInputGetStateType *)GetProcAddress(
        library, WIN32_XINPUTGETSTATE_PROCNAME
    );

    G_Win32_XInputSetStatePtr = (Win32_XInputSetStateType *)GetProcAddress(
        library, WIN32_XINPUTSETSTATE_PROCNAME
    );

    if (G_Win32_XInputSetStatePtr == NULL || G_Win32_XInputGetStatePtr == NULL)
    {
        return WIN32_LOADXINPUT_ERR;
    }

    return WIN32_LOADXINPUT_OK;
}


#define WIN32_DSOUND_DLL "dsound.dll"
#define WIN32_DIRECTSOUNDCREATE_PROCNAME "DirectSoundCreate"

typedef enum
{
    WIN32_INITDSOUND_OK,
    WIN32_INITDSOUND_ERR,
    WIN32_INITDSOUND_DLL_LOAD
} Win32_InitDSoundResult;

/*
 * Loads DirectrSound library and initializes it.
 *
 * NOTE(ilya.a): They say, that DirectSound is superseeded by WASAPI. [2024/05/25]
 * TODO(ilya.a): Check this out. [2024/05/25]
 */
internal Win32_InitDSoundResult
Win32_InitDSound(HWND window, S32 samplesPerSecond, Size bufferSize)
{
    HMODULE library = LoadLibrary(WIN32_DSOUND_DLL);

    if (library == NULL)
    {
        return WIN32_INITDSOUND_DLL_LOAD;
    }

    G_Win32_DirectSoundCreatePtr = (Win32_DirectSoundCreateType *)GetProcAddress(library, WIN32_DIRECTSOUNDCREATE_PROCNAME);

    if (G_Win32_DirectSoundCreatePtr == NULL)
    {
        return WIN32_INITDSOUND_DLL_LOAD;
    }

    LPDIRECTSOUND directSound;
    if (!SUCCEEDED(G_Win32_DirectSoundCreatePtr(0, &directSound, NULL)))
    {
        return WIN32_INITDSOUND_ERR;
    }

    if (!SUCCEEDED(VCALL(directSound, SetCooperativeLevel, window, DSSCL_PRIORITY)))
    {
        return WIN32_INITDSOUND_ERR;
    }

    // NOTE(ilya.a): Primary buffer -- buffer which is only holds handle to
    // sound card. Windows has strange API. [2024/05/25]
    DSBUFFERDESC primaryBufferDesc;
    MemoryZero(&primaryBufferDesc, sizeof(primaryBufferDesc));  // TODO(ilya.a): Checkout if we really need
                                                                // to zero buffer description. [2024/05/25]

    primaryBufferDesc.dwSize        = sizeof(primaryBufferDesc);
    primaryBufferDesc.dwFlags       = DSBCAPS_PRIMARYBUFFER;
    primaryBufferDesc.dwBufferBytes = 0;     // NOTE(ilya.a): Primary buffer size should be zero. [2024/05/25]
    primaryBufferDesc.lpwfxFormat   = NULL;  // NOTE(ilya.a): Primary buffer wfx format should be NULL. [2024/05/25]

    LPDIRECTSOUNDBUFFER primaryBuffer;
    if (!SUCCEEDED(VCALL(directSound, CreateSoundBuffer, &primaryBufferDesc, &primaryBuffer, NULL)))
    {
        return WIN32_INITDSOUND_ERR;
    }

    WAVEFORMATEX waveFormat;
    waveFormat.wFormatTag      = WAVE_FORMAT_PCM;
    waveFormat.nChannels       = 2;
    waveFormat.nSamplesPerSec  = samplesPerSecond;
    waveFormat.wBitsPerSample  = 16;
    waveFormat.nBlockAlign     = (waveFormat.nChannels * waveFormat.wBitsPerSample) / BYTE_BITS;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;  // NOTE(ilya.a): Redundant. Lol.
    waveFormat.cbSize          = 0;

    if (!SUCCEEDED(VCALL(primaryBuffer, SetFormat, &waveFormat)))
    {
        return WIN32_INITDSOUND_ERR;
    }

    // NOTE(ilya.a): Actual sound buffer in which we will write data. [2024/05/25]
    DSBUFFERDESC secondaryBufferDesc;
    MemoryZero(&secondaryBufferDesc, sizeof(secondaryBufferDesc));  // TODO(ilya.a): Checkout if we really need
                                                                    // to zero buffer description. [2024/05/25]
    secondaryBufferDesc.dwSize        = sizeof(secondaryBufferDesc);
    secondaryBufferDesc.dwFlags       = 0;
    secondaryBufferDesc.dwBufferBytes = bufferSize;
    secondaryBufferDesc.lpwfxFormat   = &waveFormat;

    if (!SUCCEEDED(VCALL(directSound, CreateSoundBuffer, &secondaryBufferDesc, &G_Win32_AudioBuffer, NULL)))
    {
        return WIN32_INITDSOUND_ERR;
    }

    return WIN32_INITDSOUND_OK;
}

typedef struct
{
    U32 runningSampleIndex;
    U32 toneHZ;
    S32 samplesPerSecond;
    S32 toneVolume;
    S32 wavePeriod;
    Size bytesPerSample;
    Size audioBufferSize;
} Win32_SoundOutput;

internal Win32_SoundOutput
Win32_SoundOutputMake(void) {
    Win32_SoundOutput ret = {0};
    ret.samplesPerSecond = 48000;
    ret.runningSampleIndex = 0;
    ret.toneHZ = 256;
    ret.toneVolume = 1000;

    ret.wavePeriod = ret.samplesPerSecond / ret.toneHZ;
    ret.bytesPerSample = sizeof(S16) * 2;
    ret.audioBufferSize = ret.samplesPerSecond * ret.bytesPerSample;
    return ret;
}

internal procedure
Win32_FillSoundBuffer(Win32_SoundOutput *soundOutput, DWORD byteToLock, DWORD bytesToWrite) {
    VOID *region1, *region2;
    Size region1Size, region2Size;

    // TODO(ilya.a): Check why it's failed to lock buffer. Sound is nice, but lock are failing [2024/07/28]
    VCALL(
        G_Win32_AudioBuffer, Lock,
        byteToLock,
        bytesToWrite,
        &region1, &region1Size,
        &region2, &region2Size,
        0
    );

    DWORD region1SampleCount = region1Size / soundOutput->bytesPerSample;
    S16 *sampleOut = (S16 *)region1;
    for (U32 sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex)
    {
        F32 sinePosition = 2.0f * PI32 * (F32)soundOutput->runningSampleIndex / (F32)soundOutput->wavePeriod;
        F32 sineValue = sinf(sinePosition);
        S16 sampleValue = (S16)(sineValue * soundOutput->toneVolume);
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
        ++soundOutput->runningSampleIndex;
    }

    DWORD region2SampleCount = region2Size / soundOutput->bytesPerSample;
    sampleOut = (S16 *)region2;
    for (U32 sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex)
    {
        F32 sinePosition = 2.0f * PI32 * (F32)soundOutput->runningSampleIndex / (F32)soundOutput->wavePeriod;
        F32 sineValue = sinf(sinePosition);
        S16 sampleValue = (S16)(sineValue * soundOutput->toneVolume);
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
        ++soundOutput->runningSampleIndex;
    }

    // TODO(ilya.a): Check for succeed. [2024/05/25]
    ASSERT_VCALL(
        G_Win32_AudioBuffer, Unlock,
        region1, region1Size,
        region2, region2Size
    );
}

LRESULT CALLBACK
Win32_MainWindowProc(HWND   window,
                     UINT   message,
                     WPARAM wParam,
                     LPARAM lParam)
{
    LRESULT result = 0;

    switch (message)
    {
        case WM_ACTIVATEAPP:
        {
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
#endif  // #if 0
        case WM_CLOSE:
        {
            // TODO(ilya.a): Ask for closing?
            OutputDebugString("T: WM_CLOSE\n");
            G_ShouldStop = true;
        } break;
        case WM_DESTROY:
        {
            // TODO(ilya.a): Casey says that we maybe should recreate window later?
            OutputDebugString("T: WM_DESTROY\n");
            // PostQuitMessage(0);
        } break;
        default:
        {
            // NOTE(ilya): Leave other events to default Window's handler.
            result = DefWindowProc(window, message, wParam, lParam);
        } break;
    }

    return result;
}


int WINAPI
WinMain(_In_ HINSTANCE instance,
        _In_opt_ HINSTANCE prevInstance,
        _In_ LPSTR commandLine,
        _In_ int showMode)
{
    UNUSED(commandLine);
    UNUSED(showMode);
    UNUSED(prevInstance);

    switch (Win32_LoadXInput())
    {
        case(WIN32_LOADXINPUT_OK):
        {
        } break;
        case (WIN32_LOADXINPUT_ERR):
        {
            OutputDebugString("E: Failed to load XInput functions!\n");
            return 0;
        } break;
        default:
        {
        } break;
    }

    persist_var LPCSTR CLASS_NAME   = "GFS";
    persist_var LPCSTR WINDOW_TITLE = "GFS";

    WNDCLASS windowClass      = {0};
    windowClass.style         = CS_VREDRAW | CS_HREDRAW;
    windowClass.lpfnWndProc   = Win32_MainWindowProc;
    windowClass.hInstance     = instance;
    windowClass.lpszClassName = CLASS_NAME;

    if (RegisterClassA(&windowClass) == 0)
    {
        OutputDebugString("E: Failed to register window class!\n");
        return 0;
    }

    HWND window = CreateWindowExA(
        0,
        windowClass.lpszClassName,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,  // int x
        CW_USEDEFAULT,  // int y
        CW_USEDEFAULT,  // int width
        CW_USEDEFAULT,  // int height
        NULL,           // windowParent
        NULL,           // menu
        instance,
        NULL
    );

    if (window == NULL)
    {
        OutputDebugString("E: Failed to initialize window!\n");
        return 0;
    }

    ShowWindow(window, showMode);

    G_Renderer = BMR_Init(COLOR_WHITE, window);
    BMR_Resize(&G_Renderer, 900, 600);

    Win32_SoundOutput soundOutput = Win32_SoundOutputMake();
    Win32_InitDSoundResult initDSoundResult = Win32_InitDSound(window, soundOutput.samplesPerSecond, soundOutput.audioBufferSize);
    if (initDSoundResult != WIN32_INITDSOUND_OK)
    {
        OutputDebugString("W: Failed to init DSound!\n");
    }

    Win32_FillSoundBuffer(&soundOutput, 0, soundOutput.audioBufferSize);
    VCALL(G_Win32_AudioBuffer, Play, 0, 0, DSBPLAY_LOOPING);

    U32 xOffset = 0;
    U32 yOffset = 0;

    G_Player.Rect.X = PLAYER_INIT_X;
    G_Player.Rect.Y = PLAYER_INIT_Y;
    G_Player.Rect.Width = PLAYER_WIDTH;
    G_Player.Rect.Height = PLAYER_HEIGHT;
    G_Player.Color = Color4Add(COLOR_RED, COLOR_BLUE);

    G_Renderer.ClearColor = COLOR_WHITE;

    while (!G_ShouldStop)
    {

        MSG message = {};
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                // NOTE(ilya.a): Make sure that we will quit the mainloop.
                G_ShouldStop = true;
            }

            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        // TODO(ilya.a): Should we pool more frequently? [2024/05/19]
        for (DWORD xControllerIndex = 0; xControllerIndex < XUSER_MAX_COUNT; ++xControllerIndex)
        {
            XINPUT_STATE xInputState = {0};
            DWORD result = G_Win32_XInputGetStatePtr(xControllerIndex, &xInputState);

            if(result == ERROR_SUCCESS)
            {
                XINPUT_GAMEPAD *pad = &xInputState.Gamepad;

                Bool dPadUp    = pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
                Bool dPadDown  = pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                Bool dPadLeft  = pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
                Bool dPadRight = pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

                G_Player.Input.RightPressed = dPadRight;
                G_Player.Input.LeftPressed = dPadLeft;

                G_Player.Input.UpPressed = dPadUp;
                G_Player.Input.DownPressed = dPadDown;

                XINPUT_VIBRATION vibrationState = {0};

                if (G_Player.Input.RightPressed || G_Player.Input.LeftPressed ||
                    G_Player.Input.UpPressed    || G_Player.Input.DownPressed)
                {
#if 0
                    vibrationState.wLeftMotorSpeed = 60000;
                    vibrationState.wRightMotorSpeed = 60000;
#endif
                }
                else
                {
                    vibrationState.wLeftMotorSpeed = 0;
                    vibrationState.wRightMotorSpeed = 0;
                }
                G_Win32_XInputSetStatePtr(
                  xControllerIndex,
                  &vibrationState
                );
            }
            else if (result == ERROR_DEVICE_NOT_CONNECTED)
            {
                // NOTE(ilya.a): Do nothing. I thought to log that.
                // But it will be too frequent log, cause we have no plans on multiple controllers.
            }
            else
            {
                OutputDebugString("E: Some of xInput's device are not connected!\n");
                return 0;
            }
        }

        if (G_Player.Input.LeftPressed)
        {
            G_Player.Rect.X -= PLAYER_SPEED;
        }

        if (G_Player.Input.RightPressed)
        {
            G_Player.Rect.X += PLAYER_SPEED;
        }

        if (G_Player.Input.DownPressed)
        {
            G_Player.Rect.Y -= PLAYER_SPEED;
        }

        if (G_Player.Input.UpPressed)
        {
            G_Player.Rect.Y += PLAYER_SPEED;
        }

        BMR_BeginDrawing(&G_Renderer);

        BMR_Clear(&G_Renderer);
        BMR_DrawGrad(&G_Renderer, xOffset, yOffset);
        BMR_DrawRectR(&G_Renderer, G_Player.Rect, G_Player.Color);
        BMR_DrawLine(&G_Renderer, 100, 200, 500, 600);

        DWORD playCursor;
        DWORD writeCursor;

        if (SUCCEEDED(VCALL(G_Win32_AudioBuffer, GetCurrentPosition, &playCursor, &writeCursor)))
        {
            DWORD byteToLock = (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) % soundOutput.audioBufferSize;
            DWORD bytesToWrite = 0;

            if (byteToLock > playCursor)
            {
                bytesToWrite = soundOutput.audioBufferSize - byteToLock;
                bytesToWrite += playCursor;
            }
            else
            {
                bytesToWrite = playCursor - byteToLock;
            }

            Win32_FillSoundBuffer(&soundOutput, byteToLock, bytesToWrite);


            if (!G_IsSoundPlaying)
            {
                ASSERT_VCALL(G_Win32_AudioBuffer, Play, 0, 0, DSBPLAY_LOOPING);
                G_IsSoundPlaying = true;
            }
        }

        BMR_EndDrawing(&G_Renderer);

        xOffset++;
        yOffset++;
    }

    BMR_DeInit(&G_Renderer);

    return 0;
}
