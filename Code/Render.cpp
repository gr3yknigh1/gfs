/*
 * FILE      Code\Render.cpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "Render.hpp"

#include "Types.hpp"
#include "Physics.hpp"
#include "Memory.hpp"
#include "Macros.hpp"
#include "Assert.hpp"

#define RENDER_COMMAND_CAPACITY 1024

Color4
Color4Add(Color4 a, Color4 b) {
    Color4 ret;
    ret.b = a.b + b.b;
    ret.g = a.g + b.g;
    ret.r = a.r + b.r;
    ret.a = a.a + b.a;
    return ret;
}

Renderer
RendererMake(PlatformWindow *window, Color4 clearColor) {
    Renderer r;

    r.clearColor = clearColor;
    r.commandQueue.begin = (byte *)PlatformMemoryAllocate(RENDER_COMMAND_CAPACITY);
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
    if (renderer->commandQueue.begin != NULL) {
        ASSERT_ISZERO(PlatformMemoryFree(renderer->commandQueue.begin));
    }
    renderer->commandQueue.begin = NULL;
    renderer->commandQueue.end = NULL;

    if (renderer->pixels.Buffer != NULL) {
        ASSERT_ISZERO(PlatformMemoryFree(renderer->pixels.Buffer));
    }
    renderer->pixels.Buffer = NULL;
}

void
BeginDrawing(Renderer *renderer) {
    UNUSED(renderer);
}

void
EndDrawing(Renderer *renderer) {
    usize pitch = renderer->pixels.Width * renderer->bytesPerPixel;
    u8 *row = (u8 *)renderer->pixels.Buffer;

    for (u32 y = 0; y < renderer->pixels.Height; ++y) {
        Color4 *pixel = (Color4 *)row;

        for (u32 x = 0; x < renderer->pixels.Width; ++x) {
            usize offset = 0;

            for (u32 commandIdx = 0; commandIdx < renderer->commandCount; ++commandIdx) {
                RenderCommandType type = *((RenderCommandType *)(renderer->commandQueue.begin + offset));

                offset += sizeof(RenderCommandType);

                switch (type) {
                case (BMR_RENDER_COMMAND_TYPE_CLEAR): {
                    Color4 color = *(Color4 *)(renderer->commandQueue.begin + offset);
                    offset += sizeof(Color4);
                    *pixel = color;
                } break;
                case (BMR_RENDER_COMMAND_TYPE_RECT): {
                    RectangleU16 rect = *(RectangleU16 *)(renderer->commandQueue.begin + offset);
                    offset += sizeof(rect);

                    Color4 color = *(Color4 *)(renderer->commandQueue.begin + offset);
                    offset += sizeof(Color4);

                    if (RectangleU16IsInside(rect, x, y)) {
                        *pixel = color;
                    }
                } break;
                case (BMR_RENDER_COMMAND_TYPE_GRADIENT): {
                    Vector2U32 v = *(Vector2U32 *)(renderer->commandQueue.begin + offset);
                    offset += sizeof(Vector2U32);

                    // TODO(ilya.a): Clamp values from U32 to U8

                    *pixel = LITERAL(Color4){.b = (u8)(x + v.x), .g = (u8)(y + v.y), .r = 0, .a = 0};
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

    RectangleI32 windowRect = PlatformWindowGetRectangle(renderer->window);
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
DrawRectangleRec(Renderer *renderer, RectangleU16 rect, Color4 color) {
    struct {
        RenderCommandType Type;
        RectangleU16 Rect;
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
