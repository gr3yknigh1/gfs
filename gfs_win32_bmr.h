/*
 * GFS. Bitmap renderer.
 *
 * Keeps one global backbuffer.
 *
 * FILE      gfs_win32_bmr.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_BMR_H_INCLUDED
#define GFS_BMR_H_INCLUDED

#include <Windows.h>

#include "gfs_types.h"
#include "gfs_color.h"
#include "gfs_linalg.h"
#include "gfs_geometry.h"

// TODO(ilya.a): Parametrize it, if will be neccesery to change bytes per pixel
#define BMR_BPP 4

/*
 * Actuall BitMap Renderer Renderer.
 */
typedef struct {
    Color4 ClearColor;

    struct {
        u8 *Begin;
        u8 *End;
    } CommandQueue;

    u64 CommandCount;

    u8 BPP;
    u64 XOffset;
    u64 YOffset;

    struct {
        void *Buffer;
        u64 Width;
        u64 Height;
    } Pixels;

    BITMAPINFO Info;
    HWND Window;
    HDC DC;
} BMR_Renderer;

typedef enum {
    BMR_RENDER_COMMAND_TYPE_NOP = 00,
    BMR_RENDER_COMMAND_TYPE_CLEAR = 01,
    BMR_RENDER_COMMAND_TYPE_LINE = 10,
    BMR_RENDER_COMMAND_TYPE_RECT = 11,
    BMR_RENDER_COMMAND_TYPE_GRADIENT = 20,
} BMR_RenderCommandType;

BMR_Renderer BMR_Init(Color4 clearColor, HWND window);
void BMR_DeInit(BMR_Renderer *renderer);

void BMR_Update(BMR_Renderer *renderer, HWND window);
void BMR_Resize(BMR_Renderer *renderer, i32 w, i32 h);

void BMR_BeginDrawing(BMR_Renderer *renderer);
void BMR_EndDrawing(BMR_Renderer *renderer);

void BMR_Clear(BMR_Renderer *renderer);

void BMR_DrawLine(BMR_Renderer *renderer, u32 x1, u32 y1, u32 x2, u32 y2);
void BMR_DrawLineV(BMR_Renderer *renderer, v2u32 p1, v2u32 p2);

void BMR_DrawRect(BMR_Renderer *renderer, u32 x, u32 y, u32 w, u32 h, Color4 c);
void BMR_DrawRectR(BMR_Renderer *renderer, Rect r, Color4 c);

void BMR_DrawGrad(BMR_Renderer *renderer, u32 xOffset, u32 yOffset);
void BMR_DrawGradV(BMR_Renderer *renderer, v2u32 offset);

#endif // GFS_BMR_H_INCLUDED
