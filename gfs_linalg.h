/*
 * FILE      gfs_linalg.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#if !defined(GFS_LINALG_H_INCLUDED)
#define GFS_LINALG_H_INCLUDED

#include "gfs_types.h"

typedef struct {
    i32 x;
    i32 y;
} Vector2I32;

typedef struct {
    u32 x;
    u32 y;
} Vector2U32;

#endif // GFS_LINALG_H_INCLUDED
