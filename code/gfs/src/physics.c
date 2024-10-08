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
GetOffsetFromCoords2DGridArrayRM(u32 width, u32 x, u32 y) {
    return width * y + x;
}

/*
 * NOTE(gr3yknigh1): Thank you SJHowe for answer on Stackoverflow. It's hard to visualize
 * my self all this flattened 3D arrays. [2024/10/08]
 *
 * Reference: <https://stackoverflow.com/a/51164817/12456897>
 *
 * Get index:
 * Row-major: Index = ((x * YSIZE + y) * ZSIZE) + z;
 * Col-major: Index = ((z * YSIZE + y) * XSIZE) + x;
 *
 * Get coords:
 * Row-major:
 * z = Index % ZSIZE;
 * y = (Index / ZSIZE) % YSIZE;
 * x = Index / (ZSIZE * YSIZE);
 *
 * Col-major:
 * x = Index % XSIZE;
 * y = (Index / XSIZE) % YSIZE;
 * z = Index / (XSIZE * YSIZE);
 *
 */

u32
GetOffsetFromCoords3DGridArrayRM(u32 width, u32 height, u32 length, u32 x, u32 y, u32 z) {
    UNUSED(width);
    return ((x * height + y) * length) + z;
}

Vector3U32
GetCoordsFrom3DGridArrayOffsetRM(u32 width, u32 height, u32 length, u32 offset) {
    UNUSED(width);

    Vector3U32 ret;
    ret.x = offset / (length * height);
    ret.y = (offset / length) % height;
    ret.z = offset % length;

    return ret;
}
