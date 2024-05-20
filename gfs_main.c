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

#include "gfs_types.h"
#include "gfs_macros.h"
#include "gfs_string.h"
#include "gfs_linalg.h"
#include "gfs_geometry.h"
#include "gfs_color.h"
#include "gfs_win32_bmr.h"
#include "gfs_win32_keys.h"
#include "gfs_win32_misc.h"


typedef DWORD Win32_XInputGetStateType(DWORD dwUserIndex, XINPUT_STATE* pState);
typedef DWORD Win32_XInputSetStateType(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

global_var Win32_XInputGetStateType *Win32_XInputGetStatePtr;
global_var Win32_XInputSetStateType *Win32_XInputSetStatePtr;


global_var BMR_Renderer renderer;
global_var bool shouldStop = false;

global_var struct
{
    Rect Rect; 
    Color4 Color;

    struct
    {
        bool LeftPressed;
        bool RightPressed;
    } Input;
} player;


#define PLAYER_INIT_X 100
#define PLAYER_INIT_Y 60
#define PLAYER_WIDTH  160
#define PLAYER_HEIGHT 80
#define PLAYER_SPEED  10


internal enum Win32_LoadXInputResult { WIN32_LOADXINPUT_OK, WIN32_LOADXINPUT_ERR }
Win32_LoadXInput(void)
{
    HMODULE library = LoadLibrary(XINPUT_DLL);

    if (library == NULL)
    {
        return WIN32_LOADXINPUT_ERR;
    }

    Win32_XInputGetStatePtr = (Win32_XInputGetStateType *)GetProcAddress(library, "XInputGetState");
    Win32_XInputSetStatePtr = (Win32_XInputSetStateType *)GetProcAddress(library, "XInputSetState");

    return WIN32_LOADXINPUT_OK;
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
#if 1
        case WM_KEYDOWN: 
        {
            switch (wParam) 
            {
                case VK_LEFT: 
                {
                    player.Input.LeftPressed = true;
                } break;
                case VK_RIGHT: 
                {
                    player.Input.RightPressed = true;
                } break;
                default: 
                {
                } break;
            }
        } break;
        case WM_KEYUP: 
        {
            switch (wParam) 
            {
                case VK_LEFT: 
                {
                    player.Input.LeftPressed = false;
                } break;
                case VK_RIGHT: 
                {
                    player.Input.RightPressed = false;
                } break;
                default: 
                {
                } break;
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

                bool dPadUp    = pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
                bool dPadDown  = pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                bool dPadLeft  = pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
                bool dPadRight = pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

                player.Input.RightPressed = dPadRight;
                player.Input.LeftPressed = dPadLeft;
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

        BMR_BeginDrawing(&renderer);

        BMR_Clear(&renderer);
        BMR_DrawGrad(&renderer, xOffset, yOffset);
        BMR_DrawRectR(&renderer, player.Rect, player.Color);

        BMR_DrawLine(&renderer, 100, 200, 500, 600);

        BMR_EndDrawing(&renderer);

        xOffset++;
        yOffset++;
    }

    BMR_DeInit(&renderer);

    return 0;
}
