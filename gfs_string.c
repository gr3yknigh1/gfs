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

Bool
CStr8IsEmpty(CStr8 s)
{
    return *s == '\0';
}

Bool
CStr8IsEqual(CStr8 s0, CStr8 s1)
{
    while (*s0 != '\0' && *s1 != '\0')
    {
        if (*s0 != *s1)
        {
            return false;
        }

        ++s0;
        ++s1;
    }
    return *s0 == '\0' && *s1 == '\0';
}
