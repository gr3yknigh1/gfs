/*
 * GFS. Bitmap renderer.
 *
 * FILE      gfs_win32_bmr.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_win32_bmr.h"

#include "gfs_types.h"
#include "gfs_linalg.h"
#include "gfs_geometry.h"
#include "gfs_memory.h"
#include "gfs_color.h"
#include "gfs_macros.h"
#include "gfs_win32_misc.h"

#define BMR_RENDER_COMMAND_CAPACITY 1024

static void
Win32_UpdateWindow(BMR_Renderer *renderer, i32 windowXOffset, i32 windowYOffset, i32 windowWidth, i32 windowHeight) {
    StretchDIBits(
        renderer->DC, windowXOffset, windowYOffset, windowWidth, windowHeight, renderer->XOffset, renderer->YOffset,
        renderer->Pixels.Width, renderer->Pixels.Height, renderer->Pixels.Buffer, &renderer->Info, DIB_RGB_COLORS,
        SRCCOPY);
}

BMR_Renderer
BMR_Init(Color4 clearColor, HWND window) {
    BMR_Renderer r;

    r.ClearColor = clearColor;
    r.CommandQueue.Begin =
        (byte *)VirtualAlloc(NULL, BMR_RENDER_COMMAND_CAPACITY, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    r.CommandQueue.End = r.CommandQueue.Begin;
    r.CommandCount = 0;

    r.BPP = BMR_BPP;
    r.XOffset = 0;
    r.YOffset = 0;

    r.Pixels.Buffer = NULL;
    r.Pixels.Width = 0;
    r.Pixels.Height = 0;

    r.Window = window;
    r.DC = GetDC(window);

    return r;
}

void
BMR_DeInit(BMR_Renderer *renderer) {
    if (renderer->CommandQueue.Begin != NULL && VirtualFree(renderer->CommandQueue.Begin, 0, MEM_RELEASE) == 0) {
        // TODO(ilya.a): Handle memory free error.
    } else {
        renderer->CommandQueue.Begin = NULL;
        renderer->CommandQueue.End = NULL;
    }

    if (renderer->Pixels.Buffer != NULL && VirtualFree(renderer->Pixels.Buffer, 0, MEM_RELEASE) == 0) {
        // TODO(ilya.a): Handle memory free error.
    } else {
        renderer->Pixels.Buffer = NULL;
    }

    ReleaseDC(renderer->Window, renderer->DC);
}

void
BMR_BeginDrawing(BMR_Renderer *renderer) {
    UNUSED(renderer);
}

void
BMR_EndDrawing(BMR_Renderer *renderer) {
    usize pitch = renderer->Pixels.Width * renderer->BPP;
    u8 *row = (u8 *)renderer->Pixels.Buffer;

    for (u64 y = 0; y < renderer->Pixels.Height; ++y) {
        Color4 *pixel = (Color4 *)row;

        for (u64 x = 0; x < renderer->Pixels.Width; ++x) {
            usize offset = 0;

            for (u64 commandIdx = 0; commandIdx < renderer->CommandCount; ++commandIdx) {
                BMR_RenderCommandType type = *((BMR_RenderCommandType *)(renderer->CommandQueue.Begin + offset));

                offset += sizeof(BMR_RenderCommandType);

                switch (type) {
                case (BMR_RENDER_COMMAND_TYPE_CLEAR): {
                    Color4 color = *(Color4 *)(renderer->CommandQueue.Begin + offset);
                    offset += sizeof(Color4);
                    *pixel = color;
                } break;
                case (BMR_RENDER_COMMAND_TYPE_LINE): {
                    v2u32 p1 = *(v2u32 *)(renderer->CommandQueue.Begin + offset);
                    offset += sizeof(v2u32);

                    v2u32 p2 = *(v2u32 *)(renderer->CommandQueue.Begin + offset);
                    offset += sizeof(v2u32);

                } break;
                case (BMR_RENDER_COMMAND_TYPE_RECT): {
                    Rect rect = *(Rect *)(renderer->CommandQueue.Begin + offset);
                    offset += sizeof(rect);

                    Color4 color = *(Color4 *)(renderer->CommandQueue.Begin + offset);
                    offset += sizeof(Color4);

                    if (RectIsInside(rect, x, y)) {
                        *pixel = color;
                    }
                } break;
                case (BMR_RENDER_COMMAND_TYPE_GRADIENT): {
                    v2u32 v = *(v2u32 *)(renderer->CommandQueue.Begin + offset);
                    offset += sizeof(v2u32);

                    *pixel = (Color4){x + v.X, y + v.Y, 0};
                } break;
                case (BMR_RENDER_COMMAND_TYPE_NOP):
                default: {
                    *pixel = renderer->ClearColor;
                } break;
                };
            }
            ++pixel;
        }

        row += pitch;
    }

    RECT windowRect;
    GetClientRect(renderer->Window, &windowRect);
    i32 x = windowRect.left;
    i32 y = windowRect.top;
    i32 width = 0, height = 0;
    Win32_GetRectSize(&windowRect, &width, &height);

    Win32_UpdateWindow(renderer, x, y, width, height);

    renderer->CommandQueue.End = renderer->CommandQueue.Begin;
    renderer->CommandCount = 0;
}

void
BMR_Update(BMR_Renderer *r, HWND window) {
    PAINTSTRUCT ps = {0};
    HDC dc = BeginPaint(window, &ps);

    if (dc == NULL) {
        // TODO(ilya.a): Handle error
    } else {
        i32 x = ps.rcPaint.left;
        i32 y = ps.rcPaint.top;
        i32 width = 0, height = 0;
        Win32_GetRectSize(&(ps.rcPaint), &width, &height);
        Win32_UpdateWindow(r, x, y, width, height);
    }

    EndPaint(window, &ps);
}

void
BMR_Resize(BMR_Renderer *r, i32 w, i32 h) {
    if (r->Pixels.Buffer != NULL && VirtualFree(r->Pixels.Buffer, 0, MEM_RELEASE) == 0) {
        //                                                           ^^^^^^^^^^^
        // NOTE(ilya.a): Might be more reasonable to use MEM_DECOMMIT instead for
        // MEM_RELEASE. Because in that case it's will be keep buffer around, until
        // we use it again.
        // P.S. Also will be good to try protect buffer after deallocating or other
        // stuff.
        //
        // TODO(ilya.a):
        //     - [ ] Checkout how it works.
        //     - [ ] Handle allocation error.
        OutputDebugString("Failed to free backbuffer memory!\n");
    }
    r->Pixels.Width = w;
    r->Pixels.Height = h;

    r->Info.bmiHeader.biSize = sizeof(r->Info.bmiHeader);
    r->Info.bmiHeader.biWidth = w;
    r->Info.bmiHeader.biHeight = h; // NOTE: Treat coordinates bottom-up. Can flip sign and make it top-down.
    r->Info.bmiHeader.biPlanes = 1;
    r->Info.bmiHeader.biBitCount = 32; // NOTE: Align to WORD
    r->Info.bmiHeader.biCompression = BI_RGB;
    r->Info.bmiHeader.biSizeImage = 0;
    r->Info.bmiHeader.biXPelsPerMeter = 0;
    r->Info.bmiHeader.biYPelsPerMeter = 0;
    r->Info.bmiHeader.biClrUsed = 0;
    r->Info.bmiHeader.biClrImportant = 0;

    usize bufferSize = w * h * r->BPP;
    r->Pixels.Buffer = VirtualAlloc(NULL, bufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    //                                                ^^^^^^^^^^^
    // TODO(ilya.a): Checkout reason why we should pass MEM_RELEASE flag. [2024/05/25]

    if (r->Pixels.Buffer == NULL) {
        // TODO:(ilya.a): Check for errors.
        OutputDebugString("Failed to allocate memory for backbuffer!\n");
    }
}

#define PUSH_RENDER_COMMAND(RENDERERPTR, PAYLOAD)                                                                      \
    do {                                                                                                               \
        MemoryCopy((RENDERERPTR)->CommandQueue.End, &(PAYLOAD), sizeof((PAYLOAD)));                                    \
        (RENDERERPTR)->CommandQueue.End += sizeof((PAYLOAD));                                                          \
        (RENDERERPTR)->CommandCount++;                                                                                 \
    } while (0)

void
BMR_Clear(BMR_Renderer *renderer) {
    struct {
        BMR_RenderCommandType Type;
        Color4 Color;
    } payload;

    payload.Type = BMR_RENDER_COMMAND_TYPE_CLEAR;
    payload.Color = renderer->ClearColor;

    PUSH_RENDER_COMMAND(renderer, payload);
}

void
BMR_DrawLine(BMR_Renderer *renderer, u32 x1, u32 y1, u32 x2, u32 y2) {
    struct {
        BMR_RenderCommandType Type;
        u32 X1;
        u32 Y1;
        u32 X2;
        u32 Y2;
    } payload;

    payload.Type = BMR_RENDER_COMMAND_TYPE_LINE;
    payload.X1 = x1;
    payload.Y1 = y1;
    payload.X2 = x2;
    payload.Y2 = y2;

    PUSH_RENDER_COMMAND(renderer, payload);
}

void
BMR_DrawLineV(BMR_Renderer *renderer, v2u32 point1, v2u32 point2) {
    struct {
        BMR_RenderCommandType Type;
        v2u32 Point1;
        v2u32 Point2;
    } payload;

    payload.Type = BMR_RENDER_COMMAND_TYPE_LINE;
    payload.Point1 = point1;
    payload.Point2 = point2;

    PUSH_RENDER_COMMAND(renderer, payload);
}

void
BMR_DrawRect(BMR_Renderer *renderer, u32 x, u32 y, u32 width, u32 height, Color4 color) {
    struct {
        BMR_RenderCommandType Type;
        u32 X;
        u32 Y;
        u32 Width;
        u32 Height;
        Color4 Color;
    } payload;

    payload.Type = BMR_RENDER_COMMAND_TYPE_RECT;
    payload.X = x;
    payload.Y = y;
    payload.Width = width;
    payload.Height = height;
    payload.Color = color;

    PUSH_RENDER_COMMAND(renderer, payload);
}

void
BMR_DrawRectR(BMR_Renderer *renderer, Rect rect, Color4 color) {
    struct {
        BMR_RenderCommandType Type;
        Rect Rect;
        Color4 Color;
    } payload;

    payload.Type = BMR_RENDER_COMMAND_TYPE_RECT;
    payload.Rect = rect;
    payload.Color = color;

    PUSH_RENDER_COMMAND(renderer, payload);
}

void
BMR_DrawGrad(BMR_Renderer *renderer, u32 xOffset, u32 yOffset) {
    struct {
        BMR_RenderCommandType Type;
        u32 XOffset;
        u32 YOffset;
    } payload;

    payload.Type = BMR_RENDER_COMMAND_TYPE_GRADIENT;
    payload.XOffset = xOffset;
    payload.YOffset = yOffset;

    PUSH_RENDER_COMMAND(renderer, payload);
}
