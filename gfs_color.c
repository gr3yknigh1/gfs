/*
 * FILE      gfs_color.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs_color.h"

Color4
Color4Add(Color4 a, Color4 b) {
    return (Color4){
        .b = a.b + b.b,
        .g = a.g + b.g,
        .r = a.r + b.r,
        .a = a.a + b.a,
    };
}
