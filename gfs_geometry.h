/*
 * FILE      gfs_geometry.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#if !defined(GFS_GEOMETRY_H_INCLUDED)
#define GFS_GEOMETRY_H_INCLUDED

#include "gfs_types.h"

typedef struct {
    u16 x;
    u16 y;
    u16 width;
    u16 height;
} RectangleU16;

bool RectangleU16IsInside(RectangleU16 r, u16 x, u16 y);
bool RectangleU16IsOverlapping(RectangleU16 r);

typedef struct {
    i32 x;
    i32 y;
    i32 width;
    i32 height;
} Rectangle32;

u64 GetOffsetForGridArray(u64 width, u64 y, u64 x);

#endif // GFS_GEOMETRY_H_INCLUDED
