/*
 * FILE      gfs_render.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs/render.h"

#include <glad/glad.h>

#include "gfs/types.h"
#include "gfs/physics.h"
#include "gfs/memory.h"
#include "gfs/macros.h"
#include "gfs/assert.h"
#include "gfs/render_opengl.h"

#define RENDER_COMMAND_CAPACITY 1024

const extern Color4RGBA COLOR4RGBA_WHITE = {1, 1, 1, 1};
const extern Color4RGBA COLOR4RGBA_BLACK = {0, 0, 0, 1};
const extern Color4RGBA COLOR4RGBA_RED = {1, 0, 0, 1};
const extern Color4RGBA COLOR4RGBA_GREEN = {0, 1, 0, 1};
const extern Color4RGBA COLOR4RGBA_BLUE = {0, 0, 1, 1};

#if 0

Renderer
RendererMake(Window *window, Color4BGRA clearColor) {
    Renderer r;

    r.clearColor = clearColor;
    r.commandQueue.begin = MemoryAllocate(RENDER_COMMAND_CAPACITY);
    r.commandQueue.end = r.commandQueue.begin;
    r.commandCount = 0;

    r.window = window;
    return r;
}

void
RendererDestroy(Renderer *renderer) {
    if (renderer->commandQueue.begin != NULL) {
        ASSERT_ISZERO(MemoryFree(renderer->commandQueue.begin, RENDER_COMMAND_CAPACITY));
    }
    renderer->commandQueue.begin = NULL;
    renderer->commandQueue.end = NULL;
}

void
BeginDrawing(Renderer *renderer) {
    UNUSED(renderer);
}

void
EndDrawing(Renderer *renderer) {

    renderer->commandQueue.end = renderer->commandQueue.begin;
    renderer->commandCount = 0;
}

#define PUSH_RENDER_COMMAND(RENDERERPTR, PAYLOAD) \
    do { \
        MemoryCopy( \
            (RENDERERPTR)->commandQueue.end, &(PAYLOAD), sizeof((PAYLOAD))); \
        (RENDERERPTR)->commandQueue.end += sizeof((PAYLOAD)); \
        (RENDERERPTR)->commandCount++; \
    } while (0)

void
ClearBackground(Renderer *renderer) {
    struct {
        RenderCommandType Type;
        Color4BGRA Color;
    } payload;

    payload.Type = BMR_RENDER_COMMAND_TYPE_CLEAR;
    payload.Color = renderer->clearColor;

    // PUSH_RENDER_COMMAND(renderer, payload);
}

void
DrawRectangle(Renderer *renderer, u32 x, u32 y, u32 width, u32 height, Color4BGRA color) {
    struct {
        RenderCommandType Type;
        u32 X;
        u32 Y;
        u32 Width;
        u32 Height;
        Color4BGRA Color;
    } payload;

    payload.Type = BMR_RENDER_COMMAND_TYPE_RECT;
    payload.X = x;
    payload.Y = y;
    payload.Width = width;
    payload.Height = height;
    payload.Color = color;

    // PUSH_RENDER_COMMAND(renderer, payload);
}

void
DrawRectangleRec(Renderer *renderer, RectangleU16 rect, Color4BGRA color) {
    struct {
        RenderCommandType Type;
        RectangleU16 Rect;
        Color4BGRA Color;
    } payload;

    payload.Type = BMR_RENDER_COMMAND_TYPE_RECT;
    payload.Rect = rect;
    payload.Color = color;

    // PUSH_RENDER_COMMAND(renderer, payload);
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

    // PUSH_RENDER_COMMAND(renderer, payload);
}

#endif // 0