/*
 * FILE      gfs_geometry.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_types.h"

#include "gfs_geometry.h"

bool
RectIsInside(Rect r, u16 x, u16 y) {
    return x >= r.X && x <= r.X + r.Width && y >= r.Y && y <= r.Y + r.Height;
}

// TODO(ilya.a): Fix bug when `r` is bigger than `this`.
bool
RectIsOverlapping(Rect r) {
    return RectIsInside(r, r.X + 0, r.Y + 0) || RectIsInside(r, r.X + r.Width, r.Y + r.Height) ||
           RectIsInside(r, r.X + r.Width, r.Y + 0) || RectIsInside(r, r.X + 0, r.Y + r.Height);
}

u64
GetOffset(u64 width, u64 y, u64 x) {
    return width * y + x;
}
