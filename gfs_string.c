/*
 * FILE      gfs_string.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs_string.h"

#include "gfs_types.h"

Size 
CStr8_GetLength(CStr8 s)
{
    Size size = 0;
    while (s[size] != '\0')
    {
        size++;
    }
    return size;
}
