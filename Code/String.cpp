/*
 * FILE      Code\String.cpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "String.hpp"

#include "Types.hpp"

usize
CString8GetLength(cstring8 s) {
    usize size = 0;
    while (s[size] != '\0') {
        size++;
    }
    return size;
}

bool
CString8IsEmpty(cstring8 s) {
    return *s == '\0';
}

bool
CString8IsEqual(cstring8 s0, cstring8 s1) {
    while (*s0 != '\0' && *s1 != '\0') {
        if (*s0 != *s1) {
            return false;
        }

        ++s0;
        ++s1;
    }
    return *s0 == '\0' && *s1 == '\0';
}
