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


#define VCALL(S, M, ...) (S)->lpVtbl->M((S), __VA_ARGS__)

typedef DWORD Win32_XInputGetStateType(DWORD dwUserIndex, XINPUT_STATE* pState);
typedef DWORD Win32_XInputSetStateType(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

typedef HRESULT Win32_DirectSoundCreateType(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

global_var Win32_XInputGetStateType *Win32_XInputGetStatePtr;
global_var Win32_XInputSetStateType *Win32_XInputSetStatePtr;

global_var Win32_DirectSoundCreateType *Win32_DirectSoundCreatePtr;

global_var LPDIRECTSOUNDBUFFER Win32_AudioBuffer;

global_var BMR_Renderer renderer;
global_var Bool shouldStop = false;

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
} player;


#define PLAYER_INIT_X 100
#define PLAYER_INIT_Y 60
#define PLAYER_WIDTH  160
#define PLAYER_HEIGHT 80
#define PLAYER_SPEED  10
 

#define WIN32_XINPUTGETSTATE_PROCNAME "XInputGetState"
#define WIN32_XINPUTSETSTATE_PROCNAME "XInputSetState"

#define WIN32_XINPUT_DLL XINPUT_DLL

internal enum Win32_LoadXInputResult { WIN32_LOADXINPUT_OK, WIN32_LOADXINPUT_ERR }
Win32_LoadXInput(void)
{
    // TODO(ilya.a): Handle different versions of xinput. Check for newer. If fails, use older one. [2024/05/24]
    HMODULE library = LoadLibrary(WIN32_XINPUT_DLL);

    if (library == NULL)
    {
        return WIN32_LOADXINPUT_ERR;
    }

    Win32_XInputGetStatePtr = (Win32_XInputGetStateType *)GetProcAddress(library, WIN32_XINPUTGETSTATE_PROCNAME);
    Win32_XInputSetStatePtr = (Win32_XInputSetStateType *)GetProcAddress(library, WIN32_XINPUTSETSTATE_PROCNAME);

    if (Win32_XInputSetStatePtr == NULL || Win32_XInputGetStatePtr == NULL)
    {
        return WIN32_LOADXINPUT_ERR;
    }

    return WIN32_LOADXINPUT_OK;
}


#define WIN32_DSOUND_DLL "dsound.dll"
#define WIN32_DIRECTSOUNDCREATE_PROCNAME "DirectSoundCreate"

/*
 * Loads DirectrSound library and initializes it.
 * 
 * NOTE(ilya.a): They say, that DirectSound is superseeded by WASAPI. [2024/05/25]
 * TODO(ilya.a): Check this out. [2024/05/25]
 */
internal enum Win32_InitDSound { WIN32_INITDSOUND_OK, WIN32_INITDSOUND_ERR, WIN32_INITDSOUND_DLL_LOAD }
Win32_InitDSound(HWND window, S32 samplesPerSecond, Size bufferSize)
{
    HMODULE library = LoadLibrary(WIN32_DSOUND_DLL);

    if (library == NULL)
    {
        return WIN32_INITDSOUND_DLL_LOAD;
    }

    Win32_DirectSoundCreatePtr = (Win32_DirectSoundCreateType *)GetProcAddress(library, WIN32_DIRECTSOUNDCREATE_PROCNAME);

    if (Win32_DirectSoundCreatePtr == NULL)
    {
        return WIN32_INITDSOUND_DLL_LOAD;
    }

    LPDIRECTSOUND directSound;
    if (!SUCCEEDED(Win32_DirectSoundCreatePtr(0, &directSound, NULL)))
    {
        return WIN32_INITDSOUND_ERR;
    }

    if (!SUCCEEDED(directSound->lpVtbl->SetCooperativeLevel(directSound, window, DSSCL_PRIORITY)))
    {
        return WIN32_INITDSOUND_ERR;
    }

    // NOTE(ilya.a): Primary buffer -- buffer which is only holds handle to sound card. Windows has strange API.
    // [2024/05/25]
    DSBUFFERDESC primaryBufferDesc;
    MemoryZero(&primaryBufferDesc, sizeof(primaryBufferDesc));  // TODO(ilya.a): Checkout if we really need 
                                                                // to zero buffer description. [2024/05/25]
    primaryBufferDesc.dwSize        = sizeof(primaryBufferDesc);
    primaryBufferDesc.dwFlags       = DSBCAPS_PRIMARYBUFFER;
    primaryBufferDesc.dwBufferBytes = 0;     // NOTE(ilya.a): Primary buffer size should be zero. [2024/05/25]
    primaryBufferDesc.lpwfxFormat   = NULL;  // NOTE(ilya.a): Primary buffer wfx format should be NULL. [2024/05/25]

    LPDIRECTSOUNDBUFFER primaryBuffer;
    if (!SUCCEEDED(directSound->lpVtbl->CreateSoundBuffer(directSound, &primaryBufferDesc, &primaryBuffer, NULL)))
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

    if (!SUCCEEDED(primaryBuffer->lpVtbl->SetFormat(primaryBuffer, &waveFormat)))
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

    if (!SUCCEEDED(directSound->lpVtbl->CreateSoundBuffer(directSound, &secondaryBufferDesc, &Win32_AudioBuffer, NULL)))
    {
        return WIN32_INITDSOUND_ERR;
    }

    return WIN32_INITDSOUND_OK;
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
            shouldStop = true;
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

    // Wave file format reading testing.
#if 1
    Arena assetArena = ArenaMake(MEGABYTES(16));
    WaveAsset waveAsset;
    WaveAssetLoadResult waLoadResult = 
        WaveAssetLoadFromFile(&assetArena, "..\\..\\Assets\\test_music_01.wav", &waveAsset);

    if (waLoadResult != WAVEASSET_LOAD_OK)
    {
        OutputDebugString("E: Failed to load wave file asset!\n");
    }
    else
    {
        /* Do something with asset */
    }

    ArenaFree(&assetArena);
#endif


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

    renderer = BMR_Init(COLOR_WHITE, window);
    BMR_Resize(&renderer, 900, 600);

    S32 samplesPerSecond     = 48000;
    U32 runningSampleIndex   = 0;
    S32 waveToneHZ           = 256 * 0.5;
    S32 waveToneVolume       = 500;
    S32 squareWavePeriod     = samplesPerSecond / waveToneHZ;
    S32 squareWaveHalfPeriod = squareWavePeriod / 2;
    Size bytesPerSample      = sizeof(S16) * 2;
    Size audioBufferSize     = samplesPerSecond * bytesPerSample;

    enum Win32_InitDSound initDSoundResult = Win32_InitDSound(window, samplesPerSecond, audioBufferSize);
    if (initDSoundResult != WIN32_INITDSOUND_OK)
    {
        OutputDebugString("W: Failed to init DSound!\n");
    }

    VCALL(Win32_AudioBuffer, Play, 0, 0, DSBPLAY_LOOPING);

    U32 xOffset = 0;
    U32 yOffset = 0;

    player.Rect.X      = PLAYER_INIT_X;
    player.Rect.Y      = PLAYER_INIT_Y;
    player.Rect.Width  = PLAYER_WIDTH;
    player.Rect.Height = PLAYER_HEIGHT;
    player.Color       = Color4Add(COLOR_RED, COLOR_BLUE);

    renderer.ClearColor = COLOR_WHITE;

    while (!shouldStop) 
    {

        MSG message = {};
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) 
        {
            if (message.message == WM_QUIT) 
            {
                // NOTE(ilya.a): Make sure that we will quit the mainloop.
                shouldStop = true;
            }

            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        // TODO(ilya.a): Should we pool more frequently? [2024/05/19]
        for (DWORD xControllerIndex = 0; xControllerIndex < XUSER_MAX_COUNT; ++xControllerIndex)
        {
            XINPUT_STATE xInputState = {0};
            DWORD result = Win32_XInputGetStatePtr(xControllerIndex, &xInputState);

            if(result == ERROR_SUCCESS)
            {
                XINPUT_GAMEPAD *pad = &xInputState.Gamepad;

                Bool dPadUp    = pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
                Bool dPadDown  = pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                Bool dPadLeft  = pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
                Bool dPadRight = pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

                player.Input.RightPressed = dPadRight;
                player.Input.LeftPressed = dPadLeft;

                player.Input.UpPressed = dPadUp;
                player.Input.DownPressed = dPadDown;

                XINPUT_VIBRATION vibrationState = {0};  
                
                if (player.Input.RightPressed || player.Input.LeftPressed || 
                    player.Input.UpPressed    || player.Input.DownPressed) 
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
                Win32_XInputSetStatePtr(
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

        if (player.Input.LeftPressed) 
        {
            player.Rect.X -= PLAYER_SPEED;
        }

        if (player.Input.RightPressed) 
        {
            player.Rect.X += PLAYER_SPEED;
        }

        if (player.Input.DownPressed) 
        {
            player.Rect.Y -= PLAYER_SPEED;
        }

        if (player.Input.UpPressed) 
        {
            player.Rect.Y += PLAYER_SPEED;
        }

        BMR_BeginDrawing(&renderer);

        BMR_Clear(&renderer);
        BMR_DrawGrad(&renderer, xOffset, yOffset);
        BMR_DrawRectR(&renderer, player.Rect, player.Color);
        BMR_DrawLine(&renderer, 100, 200, 500, 600);

        // NOTE(ilya.a): Testing DirectSound
#if 1
        DWORD playCursor;
        DWORD writeCursor;
        if (SUCCEEDED(VCALL(Win32_AudioBuffer, GetCurrentPosition, &playCursor, &writeCursor)))
        {
            DWORD byteToLock = runningSampleIndex * bytesPerSample % audioBufferSize;
            DWORD bytesToWrite;
            if (byteToLock > playCursor)
            {
                bytesToWrite = audioBufferSize - byteToLock;
                bytesToWrite += playCursor;              
            }
            else
            {
                bytesToWrite = playCursor - byteToLock;
            }

            VOID *region1,    *region2;
            Size  region1Size, region2Size;
            
            // TODO(ilya.a): Check for succeed. [2024/05/25]
            VCALL(
                Win32_AudioBuffer, Lock, 
                byteToLock,
                bytesToWrite,
                &region1, &region1Size,
                &region2, &region2Size,
                0
            );


            DWORD region1SampleCount = region1Size / bytesPerSample;
            S16 *sampleOut = (S16 *)region1;
            for (U32 sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex)
            {
                S16 sampleValue = ((runningSampleIndex / squareWaveHalfPeriod) % 2) ? waveToneVolume : -waveToneVolume;
                *sampleOut++ = sampleValue;
                *sampleOut++ = sampleValue;
                ++runningSampleIndex;
            }

            DWORD region2SampleCount = region2Size / bytesPerSample;
            sampleOut = (S16 *)region2;
            for (U32 sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex)
            {
                S16 sampleValue = ((runningSampleIndex / squareWaveHalfPeriod) % 2) ? waveToneVolume : -waveToneVolume;
                *sampleOut++ = sampleValue;
                *sampleOut++ = sampleValue;
                ++runningSampleIndex;
            }

            // TODO(ilya.a): Check for succeed. [2024/05/25]
            VCALL(
                Win32_AudioBuffer, Unlock,
                region1, region1Size,
                region2, region2Size
            );
        }
      
#endif

        BMR_EndDrawing(&renderer);

        xOffset++;
        yOffset++;
    }

    BMR_DeInit(&renderer);

    return 0;
}
