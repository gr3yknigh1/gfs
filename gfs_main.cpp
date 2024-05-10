/*
 * GFS. Entry point.
 *
 * FILE    gfs_main.hpp
 * AUTHOR  Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT Copyright (c) 2024 Ilya Akkuzin
 * 
 * This is small educational project where I
 * doodling around with Windows API and computer
 * graphics.
 *
 * Copied most of source, related to BMR from
 * my `gr3yknigh1/libsbmr` repo.
 * */

#include <Windows.h>

#include "gfs_types.hpp"
#include "gfs_string.hpp"
#include "gfs_macros.hpp"
#include "gfs_linalg.hpp"
#include "gfs_geometry.hpp"
#include "gfs_color.hpp"
#include "gfs_win32_bmr.hpp"
#include "gfs_win32_keys.hpp"
#include "gfs_win32_misc.hpp"


global_var BMR::Renderer renderer;
global_var bool shouldStop = false;

global_var struct {
    Rect Rect; 
    Color4 Color;

    struct {
        bool LeftPressed;
        bool RightPressed;
    } Input;
} player;


#define PLAYER_INIT_X 100
#define PLAYER_INIT_Y 60
#define PLAYER_WIDTH 160
#define PLAYER_HEIGHT 80
#define PLAYER_COLOR (COLOR_RED + COLOR_BLUE)
#define PLAYER_SPEED 10


LRESULT CALLBACK
Win32_MainWindowProc(HWND   window,
                     UINT   message,
                     WPARAM wParam,
                     LPARAM lParam)
{
    LRESULT result = 0;

    switch (message) {
        case WM_ACTIVATEAPP: 
        {
            OutputDebugString("WM_ACTIVATEAPP\n");
        } break;
        case WM_SIZE: 
        {
            OutputDebugString("WM_SIZE\n");
            // S32 width = LOWORD(lParam);
            // S32 height = HIWORD(lParam);
            // BMR::Resize(&renderer, width, height);
        } break;
        case WM_PAINT: 
        {
            OutputDebugString("WM_PAINT\n");
            BMR::Update(&renderer, window);
        } break;
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
        case WM_CLOSE: 
        {
            // TODO(ilya.a): Ask for closing?
            OutputDebugString("WM_CLOSE\n");
            shouldStop = true;
        } break;
        case WM_DESTROY: 
        {
            // TODO(ilya.a): Casey says that we maybe should recreate
            // window later?
            OutputDebugString("WM_DESTROY\n");
            // PostQuitMessage(0);
        } break;
        default: 
        {
            // Leave other events to default Window's handler.
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

    renderer = BMR::Init();
    BMR::Resize(&renderer, 900, 600);

    persist_var LPCSTR CLASS_NAME = "GFS";
    persist_var LPCSTR WINDOW_TITLE = "GFS";

    WNDCLASS windowClass = {0};
    windowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    windowClass.lpfnWndProc = Win32_MainWindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = CLASS_NAME;

    if (RegisterClassA(&windowClass) == 0) 
    {
        OutputDebugString("Failed to register window class!\n");
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
        nullptr,        // windowParent
        nullptr,        // menu
        instance,
        nullptr
    );

    if (window == nullptr) 
    {
        OutputDebugString("Failed to initialize window!\n");
        return 0;
    }

    ShowWindow(window, showMode);

    U32 xOffset = 0;
    U32 yOffset = 0;

    player.Rect.X = PLAYER_INIT_X;
    player.Rect.Y = PLAYER_INIT_Y;
    player.Rect.Width = PLAYER_WIDTH;
    player.Rect.Height = PLAYER_HEIGHT;
    player.Color = PLAYER_COLOR;

    renderer.ClearColor = COLOR_WHITE;

    while (!shouldStop) 
    {

        MSG message = {};
        while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) 
        {
            if (message.message == WM_QUIT) 
            {
                // NOTE(ilya.a): Make sure that we will quit the mainloop.
                shouldStop = true;
            }

            TranslateMessage(&message);
            DispatchMessageA(&message);
        }


        if (player.Input.LeftPressed) 
        {
            player.Rect.X -= PLAYER_SPEED;
        }

        if (player.Input.RightPressed) 
        {
            player.Rect.X += PLAYER_SPEED;
        }

        BMR::BeginDrawing(&renderer, window);

        BMR::Clear(&renderer);
        BMR::DrawGrad(&renderer, xOffset, yOffset);
        BMR::DrawRect(&renderer, player.Rect, player.Color);

        BMR::DrawLine(&renderer, 100, 200, 500, 600);

        BMR::EndDrawing(&renderer);

        xOffset++;
        yOffset++;
    }

    BMR::DeInit(&renderer);

    return 0;
}
