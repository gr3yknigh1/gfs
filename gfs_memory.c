/*
 * FILE      gfs_memory.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_memory.h"

// TODO(ilya.a): Use SIMD [2024/05/19]

void 
MemoryCopy(void *destination, const void *source, Size size) 
{
    for (Size i = 0; i < size; ++i) 
    {
        ((Byte *)destination)[i] = ((Byte *)source)[i];
    }
}

void 
MemorySet(void *data, Byte value, Size size)
{
    for (Size i = 0; i < size; ++i) 
    {
        ((Byte *)data)[i] = value;
    }
}

void 
MemoryZero(void *data, Size size)
{
    for (Size i = 0; i < size; ++i) 
    {
        ((Byte *)data)[i] = 0;
    }
}
