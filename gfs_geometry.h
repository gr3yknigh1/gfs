/*
 * FILE      gfs_geometry.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_GEOMETRY_H_INCLUDED
#define GFS_GEOMETRY_H_INCLUDED

#include "gfs_types.h"

typedef struct {
    U16 X;
    U16 Y;
    U16 Width;
    U16 Height;
} Rect;

Bool RectIsInside(Rect r, U16 x, U16 y);
Bool RectIsOverlapping(Rect r);

U64 GetOffset(U64 width, U64 y, U64 x);

#endif // GFS_GEOMETRY_H_INCLUDED
