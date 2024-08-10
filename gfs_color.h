/*
 * FILE      gfs_color.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_COLOR_H_INCLUDED
#define GFS_COLOR_H_INCLUDED

#include "gfs_types.h"
#include "gfs_macros.h"

// NOTE(ilya.a): Ordered according to GDI requirements.
typedef struct {
    u8 b;
    u8 g;
    u8 r;
    u8 a;
} Color4;

Color4 Color4Add(Color4 a, Color4 b);

#define COLOR_WHITE                                                                                                    \
    (Color4) { U8_MAX, U8_MAX, U8_MAX, U8_MAX }
#define COLOR_RED                                                                                                      \
    (Color4) { U8_MAX, 0, 0, 0 }
#define COLOR_GREEN                                                                                                    \
    (Color4) { 0, U8_MAX, 0, 0 }
#define COLOR_BLUE                                                                                                     \
    (Color4) { 0, 0, U8_MAX, 0 }
#define COLOR_BLACK                                                                                                    \
    (Color4) { 0, 0, 0, 0 }

#endif // GFS_COLOR_H_INCLUDED
