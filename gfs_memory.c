/*
 * FILE      gfs_memory.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_memory.h"


void 
MemoryCopy(void *destination, const void *source, Size size) {
    // TODO(ilya.a): Use SIMD [2024/05/19]

    for (Size i = 0; i < size; ++i) {
        ((Byte *)destination)[i] = ((Byte *)source)[i];
    }
}
