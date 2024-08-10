/*
 * FILE      gfs_color.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs_color.h"

Color4
Color4Add(Color4 a, Color4 b) {
    return (Color4){
        .B = a.B + b.B,
        .G = a.G + b.G,
        .R = a.R + b.R,
        .A = a.A + b.A,
    };
}
