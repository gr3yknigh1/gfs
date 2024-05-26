/*
 * FILE      gfs_memory.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_MEMORY_H_INCLUDED
#define GFS_MEMORY_H_INCLUDED

#include "gfs_types.h"

/*
 * Arena Allocator (aka bump allocator).
 */
typedef struct {
    Void *Begin;
    Void *End;
    Void *AllocPos;
} Arena;

Arena ArenaMake(Size size);
Arena ArenaMakeA(Arena *arena, Size size);
Arena ArenaMakeEx(Void *begin, Size size);

Void *ArenaAlloc(Arena *arena, Size size);

void ArenaFree(Arena *arena);

void MemoryCopy(Void *dest, const Void *source, Size size);
void MemorySet(Void *data, Byte value, Size size);
void MemoryZero(Void *data, Size size);

#endif  // GFS_MEMORY_H_INCLUDED
