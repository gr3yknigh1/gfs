#if !defined(GFS_PHYSICS_H_INCLUDED)
#define GFS_PHYSICS_H_INCLUDED
/*
 * FILE      gfs_physics.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs/types.h"

/*
 * @breaf Gets offset from beginning of the array, as if this array represents
 * grid.
 */
u64 GetOffsetForGridArray(u64 width, u64 y, u64 x);

typedef struct {
    i32 x;
    i32 y;
} Vector2I32;

typedef struct {
    u32 x;
    u32 y;
} Vector2U32;

typedef struct {
    f32 x;
    f32 y;
} Vector2F32;

typedef Vector2F32 Vector2;

typedef struct {
    u16 x;
    u16 y;
    u16 width;
    u16 height;
} RectangleU16;

typedef struct {
    f32 x;
    f32 y;
    f32 width;
    f32 height;
} RectangleF32;

bool RectangleU16IsInside(RectangleU16 r, u16 x, u16 y);
bool RectangleU16IsOverlapping(RectangleU16 r);

typedef struct {
    i32 x;
    i32 y;
    i32 width;
    i32 height;
} RectangleI32;

#endif // GFS_PHYSICS_H_INCLUDED
