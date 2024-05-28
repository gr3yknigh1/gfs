/*
 * FILE      gfs_string.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_STRING_H_INCLUDED
#define GFS_STRING_H_INCLUDED

#include "gfs_types.h"

typedef struct {
    Char8 *Data;
    Size   Size;
} Str8;

Size CStr8GetLength(CStr8 s);
Bool CStr8IsEmpty(CStr8 s);
Bool CStr8IsEqual(CStr8 s0, CStr8 s1);

#endif  // GFS_STRING_H_INCLUDED
