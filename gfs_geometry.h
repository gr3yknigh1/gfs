/*
 * FILE      gfs_geometry.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_GEOMETRY_H_INCLUDED
#define GFS_GEOMETRY_H_INCLUDED

#include "gfs_types.h"

typedef struct {
    u16 X;
    u16 Y;
    u16 Width;
    u16 Height;
} Rect;

typedef struct {
    i32 x;
    i32 y;
    i32 width;
    i32 height;
} Rect32;

bool RectIsInside(Rect r, u16 x, u16 y);
bool RectIsOverlapping(Rect r);

u64 GetOffset(u64 width, u64 y, u64 x);

#endif // GFS_GEOMETRY_H_INCLUDED
