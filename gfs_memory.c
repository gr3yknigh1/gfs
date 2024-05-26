/*
 * FILE      gfs_memory.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_memory.h"

#include <Windows.h>

#include "gfs_types.h"


Arena
ArenaMake(Size size)
{
    Void * data = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    //                               ^^^^
    // NOTE(ilya.a): So, here I am reserving `size` amount of bytes, but accually `VirtualAlloc`
    // will round up this number to next page. [2024/05/26]
    // TODO(ilya.a): Do something about waste of unused memory in Arena. [2024] 

    return (Arena) 
    {
        .Data     = data,
        .Capacity = size,
        .Occupied = 0,
    };
}

Void *
ArenaAlloc(Arena *arena, Size size)
{
    if (arena == NULL || arena->Data == NULL)
    {
        return NULL;
    }

    if (arena->Occupied + size > arena->Capacity)
    {
        return NULL;
    }

    Void *data = ((Byte *)arena->Data) + arena->Occupied;
    arena->Occupied += size;
    return data;
}

void 
ArenaFree(Arena *arena)
{
    if (arena == NULL || arena->Data == NULL)
    {
        return;
    }

    VirtualFree(arena->Data, 0, MEM_RELEASE);

    arena->Data = NULL;
    arena->Capacity = 0;
    arena->Occupied = 0;
}


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
