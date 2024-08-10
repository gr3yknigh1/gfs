/*
 * FILE      gfs_win32_misc.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_WIN32_MISC_HPP_INCLUDED
#define GFS_WIN32_MISC_HPP_INCLUDED

#include <Windows.h>

#include "gfs_macros.h"
#include "gfs_types.h"
#include "gfs_string.h"

/*
 * Extracts width and height of Win32's `RECT` type.
 */
void Win32_GetRectSize(const RECT *r, i32 *w, i32 *h);

#define Win32_TextOutA_CString8(HDC, X, Y, MSG) TextOutA((HDC), (X), (Y), (MSG), CStr_GetLength((MSG)))

#endif // GFS_WIN32_MISC_HPP_INCLUDED
