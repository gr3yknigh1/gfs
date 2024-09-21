#if !defined(GFS_RENDER_H_INCLUDED)
/*
 * FILE      gfs_render.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_RENDER_H_INCLUDED

#include "gfs_types.h"
#include "gfs_physics.h"
#include "gfs_platform.h"

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

#endif // GFS_RENDER_H_INCLUDED
