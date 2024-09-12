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
} Color4;

Color4 Color4Add(Color4 a, Color4 b);

#define COLOR_WHITE                                                                                                    \
    (Color4) { 255, 255, 255, 255 }
#define COLOR_RED                                                                                                      \
    (Color4) { 255, 0, 0, 0 }
#define COLOR_GREEN                                                                                                    \
    (Color4) { 0, 255, 0, 0 }
#define COLOR_BLUE                                                                                                     \
    (Color4) { 0, 0, 255, 0 }
#define COLOR_BLACK                                                                                                    \
    (Color4) { 0, 0, 0, 0 }

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
