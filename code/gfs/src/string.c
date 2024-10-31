/*
 * FILE      gfs_string.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs/string.h"

#include "gfs/types.h"

StringView
StringViewFromCString(cstring8 s)
{
    StringView sv = {0};
    sv.buffer = s;
    sv.length = CString8GetLength(s);
    return sv;
}

usize
CString8GetLength(cstring8 s)
{
    usize size = 0;
    while (s[size] != '\0') {
        size++;
    }
    return size;
}

bool
CString8IsEmpty(cstring8 s)
{
    return *s == '\0';
}

bool
CString8IsEqual(cstring8 s0, cstring8 s1)
{
    while (*s0 != '\0' && *s1 != '\0') {
        if (*s0 != *s1) {
            return false;
        }

        ++s0;
        ++s1;
    }
    return *s0 == '\0' && *s1 == '\0';
}
