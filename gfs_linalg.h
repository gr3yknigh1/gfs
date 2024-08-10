/*
 * FILE      gfs_linalg.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#if !defined(GFS_LINALG_H_INCLUDED)
#define GFS_LINALG_H_INCLUDED

#include "gfs_types.h"

typedef struct {
    i32 X;
    i32 Y;
} v2i32;

typedef struct {
    u32 X;
    u32 Y;
} v2u32;

#endif // GFS_LINALG_H_INCLUDED
