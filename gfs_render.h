/*
 * FILE      gfs_render.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#if !defined(GFS_RENDER_H_INCLUDED)
#define GFS_RENDER_H_INCLUDED

#include <Windows.h>

#include "gfs_types.h"
#include "gfs_color.h"
#include "gfs_physics.h"
#include "gfs_platform.h"

/*
 * Actuall BitMap Renderer Renderer.
 */
typedef struct {
    Color4 clearColor;

    struct {
        u8 *begin;
        u8 *end;
    } commandQueue;

    u64 commandCount;

    u8 bytesPerPixel;
    u64 xOffset;
    u64 yOffset;

    struct {
        void *Buffer;
        u64 Width;
        u64 Height;
    } pixels;

    PlatformWindow *window;
} Renderer;

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

#endif // GFS_RENDER_H_INCLUDED
