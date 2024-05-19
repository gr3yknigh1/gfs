/*
 * FILE      gfs_win32_misc.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_win32_misc.h"

void
Win32_GetRectSize(
    const RECT *r,
    S32        *w,
    S32        *h)
{
    *w = r->right  - r->left;
    *h = r->bottom - r->top;
}
