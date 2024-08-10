/*
 * FILE      gfs_string.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs_string.h"

#include "gfs_types.h"

usize
CStr8_GetLength(cstr8 s) {
    usize size = 0;
    while (s[size] != '\0') {
        size++;
    }
    return size;
}

bool
CStr8IsEmpty(cstr8 s) {
    return *s == '\0';
}

bool
CStr8IsEqual(cstr8 s0, cstr8 s1) {
    while (*s0 != '\0' && *s1 != '\0') {
        if (*s0 != *s1) {
            return false;
        }

        ++s0;
        ++s1;
    }
    return *s0 == '\0' && *s1 == '\0';
}
