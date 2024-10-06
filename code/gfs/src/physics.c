/*
 * FILE      gfs_physics.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs/physics.h"
#include "gfs/types.h"

bool
RectangleU16IsInside(RectangleU16 r, u16 x, u16 y) {
    return x >= r.x && x <= r.x + r.width && y >= r.y && y <= r.y + r.height;
}

// TODO(ilya.a): Fix bug when `r` is bigger than `this`.
bool
RectangleU16IsOverlapping(RectangleU16 r) {
    return RectangleU16IsInside(r, r.x + 0, r.y + 0) || RectangleU16IsInside(r, r.x + r.width, r.y + r.height) ||
           RectangleU16IsInside(r, r.x + r.width, r.y + 0) || RectangleU16IsInside(r, r.x + 0, r.y + r.height);
}

u32
GetOffsetFromCoords2DGridArray(u32 width, u32 x, u32 y) {
    return width * y + x;
}

// GetOffsetFromCoordsGrid3DArray
// GetCoordsFrom3DGridArrayOffset