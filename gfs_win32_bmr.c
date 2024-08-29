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

#define RENDER_COMMAND_CAPACITY 1024


Renderer
RendererMake(PlatformWindow *window, Color4 clearColor) {
    Renderer r;

    r.clearColor = clearColor;
    r.commandQueue.begin = PlatformMemoryAllocate(RENDER_COMMAND_CAPACITY);
    r.commandQueue.end = r.commandQueue.begin;
    r.commandCount = 0;

    r.bytesPerPixel = 4;
    r.xOffset = 0;
    r.yOffset = 0;

    r.pixels.Buffer = NULL;
    r.pixels.Width = 0;
    r.pixels.Height = 0;

    r.window = window;
    return r;
}

void
RendererDestroy(Renderer *renderer) {
    if (renderer->commandQueue.begin != NULL && VirtualFree(renderer->commandQueue.begin, 0, MEM_RELEASE) == 0) {
        // TODO(ilya.a): Handle memory free error.
    } else {
        renderer->commandQueue.begin = NULL;
        renderer->commandQueue.end = NULL;
    }

    if (renderer->pixels.Buffer != NULL && VirtualFree(renderer->pixels.Buffer, 0, MEM_RELEASE) == 0) {
        // TODO(ilya.a): Handle memory free error.
    } else {
        renderer->pixels.Buffer = NULL;
    }
}

void
BeginDrawing(Renderer *renderer) {
    UNUSED(renderer);
}

void
EndDrawing(Renderer *renderer) {
    usize pitch = renderer->pixels.Width * renderer->bytesPerPixel;
    u8 *row = (u8 *)renderer->pixels.Buffer;

    for (u64 y = 0; y < renderer->pixels.Height; ++y) {
        Color4 *pixel = (Color4 *)row;

        for (u64 x = 0; x < renderer->pixels.Width; ++x) {
            usize offset = 0;

            for (u64 commandIdx = 0; commandIdx < renderer->commandCount; ++commandIdx) {
                RenderCommandType type = *((RenderCommandType *)(renderer->commandQueue.begin + offset));

                offset += sizeof(RenderCommandType);

                switch (type) {
                case (BMR_RENDER_COMMAND_TYPE_CLEAR): {
                    Color4 color = *(Color4 *)(renderer->commandQueue.begin + offset);
                    offset += sizeof(Color4);
                    *pixel = color;
                } break;
                case (BMR_RENDER_COMMAND_TYPE_RECT): {
                    Rect rect = *(Rect *)(renderer->commandQueue.begin + offset);
                    offset += sizeof(rect);

                    Color4 color = *(Color4 *)(renderer->commandQueue.begin + offset);
                    offset += sizeof(Color4);

                    if (RectIsInside(rect, x, y)) {
                        *pixel = color;
                    }
                } break;
                case (BMR_RENDER_COMMAND_TYPE_GRADIENT): {
                    v2u32 v = *(v2u32 *)(renderer->commandQueue.begin + offset);
                    offset += sizeof(v2u32);

                    *pixel = (Color4){
                        .b=x + v.X,
                        .g=y + v.Y,
                        .r=0,
                        .a=0
                    };
                } break;
                case (BMR_RENDER_COMMAND_TYPE_NOP):
                default: {
                    *pixel = renderer->clearColor;
                } break;
                };
            }
            ++pixel;
        }

        row += pitch;
    }

    Rect32 windowRect = PlatformWindowGetRectangle(renderer->window);
    PlatformWindowUpdate(renderer->window, windowRect.x, windowRect.y, windowRect.width, windowRect.height);

    renderer->commandQueue.end = renderer->commandQueue.begin;
    renderer->commandCount = 0;
}

#define PUSH_RENDER_COMMAND(RENDERERPTR, PAYLOAD)                                                                      \
    do {                                                                                                               \
        MemoryCopy((RENDERERPTR)->commandQueue.end, &(PAYLOAD), sizeof((PAYLOAD)));                                    \
        (RENDERERPTR)->commandQueue.end += sizeof((PAYLOAD));                                                          \
        (RENDERERPTR)->commandCount++;                                                                                 \
    } while (0)

void
ClearBackground(Renderer *renderer) {
    struct {
        RenderCommandType Type;
        Color4 Color;
    } payload;

    payload.Type = BMR_RENDER_COMMAND_TYPE_CLEAR;
    payload.Color = renderer->clearColor;

    PUSH_RENDER_COMMAND(renderer, payload);
}

void
DrawRectangle(Renderer *renderer, u32 x, u32 y, u32 width, u32 height, Color4 color) {
    struct {
        RenderCommandType Type;
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
DrawRectangleRec(Renderer *renderer, Rect rect, Color4 color) {
    struct {
        RenderCommandType Type;
        Rect Rect;
        Color4 Color;
    } payload;

    payload.Type = BMR_RENDER_COMMAND_TYPE_RECT;
    payload.Rect = rect;
    payload.Color = color;

    PUSH_RENDER_COMMAND(renderer, payload);
}

void
DrawGradient(Renderer *renderer, u32 xOffset, u32 yOffset) {
    struct {
        RenderCommandType Type;
        u32 XOffset;
        u32 YOffset;
    } payload;

    payload.Type = BMR_RENDER_COMMAND_TYPE_GRADIENT;
    payload.XOffset = xOffset;
    payload.YOffset = yOffset;

    PUSH_RENDER_COMMAND(renderer, payload);
}
