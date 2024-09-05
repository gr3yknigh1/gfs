#pragma once
/*
 * FILE      Render.hpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "Types.hpp"
#include "Physics.hpp"
#include "Platform.hpp"

// NOTE(ilya.a): Ordered according to GDI requirements.
// TODO(ilya.a): Replace ordering with RGA order.
struct Color4 {
    u8 b;
    u8 g;
    u8 r;
    u8 a;
};

Color4 Color4Add(Color4 a, Color4 b);

#define COLOR_WHITE                                                                                                    \
    LITERAL(Color4) { 255, 255, 255, 255 }
#define COLOR_RED                                                                                                      \
    LITERAL(Color4) { 255, 0, 0, 0 }
#define COLOR_GREEN                                                                                                    \
    LITERAL(Color4) { 0, 255, 0, 0 }
#define COLOR_BLUE                                                                                                     \
    LITERAL(Color4) { 0, 0, 255, 0 }
#define COLOR_BLACK                                                                                                    \
    LITERAL(Color4) { 0, 0, 0, 0 }

/*
 * Actuall BitMap Renderer Renderer.
 */
struct Renderer {
    Color4 clearColor;

    struct {
        byte *begin;
        byte *end;
    } commandQueue;

    u32 commandCount;

    u8 bytesPerPixel;
    u64 xOffset;
    u64 yOffset;

    struct {
        void *Buffer;
        u32 Width;
        u32 Height;
    } pixels;

    PlatformWindow *window;
};

typedef enum {
    BMR_RENDER_COMMAND_TYPE_NOP = 00,
    BMR_RENDER_COMMAND_TYPE_CLEAR = 01,
    BMR_RENDER_COMMAND_TYPE_RECT = 11,
    BMR_RENDER_COMMAND_TYPE_GRADIENT = 20,
} RenderCommandType;

Renderer RendererMake(PlatformWindow *window, Color4 clearColor);
void RendererDestroy(Renderer *renderer);

void BeginDrawing(Renderer *renderer);
void EndDrawing(Renderer *renderer);

void ClearBackground(Renderer *renderer);

void DrawRectangle(Renderer *renderer, u32 x, u32 y, u32 w, u32 h, Color4 c);
void DrawRectangleRec(Renderer *renderer, RectangleU16 r, Color4 c);

void DrawGradient(Renderer *renderer, u32 xOffset, u32 yOffset);
