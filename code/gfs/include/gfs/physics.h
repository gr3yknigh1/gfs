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

/*
 * @breaf Gets offset from beginning of the array, as if this array represents
 * grid.
 */
GFS_API u32 GetOffsetFromCoords2DGridArray(u32 width, u32 x, u32 y);


// #define chunk_pos_to_index(p) (p.x * CHUNK_SIZE.x * CHUNK_SIZE.z + p.z * CHUNK_SIZE.z + p.y)

GFS_API u32 GetOffsetFromCoordsGrid3DArray(/**/);
GFS_API Vector3U32 GetCoordsFrom3DGridArrayOffset(/**/);

GFS_API bool RectangleU16IsInside(RectangleU16 r, u16 x, u16 y);
GFS_API bool RectangleU16IsOverlapping(RectangleU16 r);

#endif // GFS_PHYSICS_H_INCLUDED
