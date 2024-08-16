/*
 * FILE      gfs_win32.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs_assert.h"
#include "gfs_platform.h"
#include "gfs_types.h"
#include "gfs_memory.h"
#include "gfs_state.h"
#include "gfs_game.h"
#include "gfs_string.h"
#include "gfs_macros.h"

#include "gfs_win32_bmr.h"

#include <Windows.h>

static BMR_Renderer gRenderer;

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
        StateStop();
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

    GameMainloop();
}

typedef struct PlatformWindow {
    HWND windowHandle;
    WNDCLASS windowClass;
} PlatformWindow;

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
    ASSERT_NONNULL(window);

    ShowWindow(window->windowHandle, SW_SHOW);

    gRenderer = BMR_Init(COLOR_WHITE, window->windowHandle);
    BMR_Resize(&gRenderer, width, height);

    return window;
}

void
PlatformPoolEvents(PlatformWindow *window) {
    UNUSED(window);

    MSG message = {};
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) {
            // NOTE(ilya.a): Make sure that we will quit the mainloop.
            StateStop();
        }

        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}

void
PlatformWindowClose(PlatformWindow *window) {
    BMR_DeInit(&gRenderer);
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

void
PlatformMemoryFree(void *data) {
    VirtualFree(data, 0, MEM_RELEASE);
}
