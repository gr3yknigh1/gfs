/*
 * GFS
 *
 * FILE    gfs_win32_misc.hpp
 * AUTHOR  Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE Copyright (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_WIN32_MISC_HPP_INCLUDED
#define GFS_WIN32_MISC_HPP_INCLUDED

#include <Windows.h>

#include "gfs_macros.hpp"
#include "gfs_types.hpp"
#include "gfs_string.hpp"

/*
 * Extracts width and height of Win32's `RECT` type.
 */
constexpr void
GetRectSize(In  const RECT *r,
            Out S32        *w,
            Out S32        *h) noexcept
{
    *w = r->right  - r->left;
    *h = r->bottom - r->top;
}

#define Win32_TextOutA_CStr(HDC, X, Y, MSG) TextOutA((HDC), (X), (Y), (MSG), CStr_GetLength((MSG)))

#endif  // GFS_WIN32_MISC_HPP_INCLUDED
