/*
 * FILE      gfs_string.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_STRING_H_INCLUDED
#define GFS_STRING_H_INCLUDED

#include "gfs_types.h"

usize CString8GetLength(cstring8 s);
bool CString8IsEmpty(cstring8 s);
bool CString8IsEqual(cstring8 s0, cstring8 s1);

#endif // GFS_STRING_H_INCLUDED
