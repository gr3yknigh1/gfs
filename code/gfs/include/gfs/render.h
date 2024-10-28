#if !defined(GFS_RENDER_H_INCLUDED)
/*
 * FILE      gfs_render.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_RENDER_H_INCLUDED

#include "gfs/types.h"
#include "gfs/physics.h"
#include "gfs/platform.h"

// NOTE(ilya.a): Ordered according to GDI requirements.
// TODO(ilya.a): Replace ordering with RGA order.
typedef struct {
    u8 b;
    u8 g;
    u8 r;
    u8 a;
} Color4BGRA;

typedef struct {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Color4RGBA;

typedef struct {
    u8 r;
    u8 g;
    u8 b;
} Color3RGB;

const extern Color4RGBA COLOR4RGBA_WHITE;
const extern Color4RGBA COLOR4RGBA_BLACK;
const extern Color4RGBA COLOR4RGBA_RED;
const extern Color4RGBA COLOR4RGBA_GREEN;
const extern Color4RGBA COLOR4RGBA_BLUE;

#if 0

/*
 * @breaf Actual BitMap Renderer Renderer.
 */
typedef struct {
    Color4BGRA clearColor;

    struct {
        u8 *begin;
        u8 *end;
    } commandQueue;
    u64 commandCount;

    Window *window;
} Renderer;

typedef enum {
    BMR_RENDER_COMMAND_TYPE_NOP = 00,
    BMR_RENDER_COMMAND_TYPE_CLEAR = 01,
    BMR_RENDER_COMMAND_TYPE_RECT = 11,
    BMR_RENDER_COMMAND_TYPE_GRADIENT = 20,
} RenderCommandType;

Renderer RendererMake(Window *window, Color4BGRA clearColor);
void RendererDestroy(Renderer *renderer);

void BeginDrawing(Renderer *renderer);
void EndDrawing(Renderer *renderer);

void ClearBackground(Renderer *renderer);

void DrawRectangle(Renderer *renderer, u32 x, u32 y, u32 w, u32 h, Color4BGRA c);
void DrawRectangleRec(Renderer *renderer, RectangleU16 r, Color4BGRA c);

void DrawGradient(Renderer *renderer, u32 xOffset, u32 yOffset);

#endif // 0

#endif // GFS_RENDER_H_INCLUDED
