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
    void *Begin;
    void *End;
    void *AllocPos;
} Arena;

Arena ArenaMake(Size size);
Arena ArenaMakeA(Arena *arena, Size size);
Arena ArenaMakeEx(void *begin, Size size);

void *ArenaAlloc(Arena *arena, Size size);

void ArenaFree(Arena *arena);

void MemoryCopy(void *destination, const void *source, Size size);

#endif  // GFS_MEMORY_H_INCLUDED
