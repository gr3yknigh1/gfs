#if !defined(GFS_PHYSICS_H_INCLUDED)
#define GFS_PHYSICS_H_INCLUDED
/*
 * FILE      gfs\code\gfs\include\gfs\physics.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs/types.h"
#include "gfs/macros.h"

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
    u32 x;
    u32 y;
    u32 z;
} Vector3U32;

typedef struct {
    i32 x;
    i32 y;
    i32 z;
} Vector3I32;

typedef struct {
    u64 x;
    u64 y;
    u64 z;
} Vector3U64;

typedef struct {
    f32 x;
    f32 y;
    f32 z;
} Vector3F32;

typedef Vector3F32 Vector3;

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

typedef struct {
    i32 x;
    i32 y;
    i32 width;
    i32 height;
} RectangleI32;

// TODO(gr3yknigh1): Maybe inline them? [2024/10/08]

/*
 * @breaf Gets offset from beginning of the array, as if this flattened array
 * represents 2D grid in row-major order.
 *
 * @param width Represents count of items in one row. Max X value.
 * @param x Self explanatory
 * @param y Self explanatory
 */
GFS_API u32 GetOffsetFromCoords2DGridArrayRM(u32 width, u32 x, u32 y);

/*
 * @breaf Gets offset from beginning of the array, as if this flattened array
 * represents 3D grid in row-major order.
 *
 * @param width Represents count of items in one row. Max X value.
 * @param height Represents count of rows in one column. Max Y value.
 * @param x Self explanatory
 * @param y Self explanatory
 */
GFS_API i32 GetOffsetFromCoords3DGridArrayRM(
    i32 width, i32 height, i32 length, i32 x, i32 y, i32 z);

/*
 * @breaf Gets coordinates (x,y,z) from index of flattened 3D array (row-major
 * order).
 */
GFS_API Vector3U32
GetCoordsFrom3DGridArrayOffsetRM(i32 width, i32 height, i32 length, i32 offset);

GFS_API bool RectangleU16IsInside(RectangleU16 r, u16 x, u16 y);
GFS_API bool RectangleU16IsOverlapping(RectangleU16 r);

GFS_API f32
MapU32ValueRangeToF32(u32 value, u32 inMin, u32 inMax, f32 outMin, f32 outMax);

#endif // GFS_PHYSICS_H_INCLUDED
