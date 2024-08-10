/*
 * FILE      gfs_string.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_STRING_H_INCLUDED
#define GFS_STRING_H_INCLUDED

#include "gfs_types.h"

typedef struct {
    char8 *Data;
    usize Size;
} Str8;

usize CStr8GetLength(cstr8 s);
bool CStr8IsEmpty(cstr8 s);
bool CStr8IsEqual(cstr8 s0, cstr8 s1);

#endif // GFS_STRING_H_INCLUDED
