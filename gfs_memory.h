/*
 * FILE      gfs_memory.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_MEMORY_H_INCLUDED
#define GFS_MEMORY_H_INCLUDED

#include "gfs_types.h"
#include "gfs_macros.h"

#define KILOBYTES(X) (1024 * (X))
#define MEGABYTES(X) (1024 * 1024 * (X))
#define GIGABYTES(X) (1024 * 1024 * 1024 * (X))

Size Align2PageSize(Size size);

/*
 * Scratch Allocator.
 */
typedef struct {
    Void *Data;
    Size Capacity;
    Size Occupied;
} ScratchAllocator;

#define SCRATCH_ALLOCATOR_HAS_SPACE(ALLOCATORPTR, SIZE) ((ALLOCATORPTR)->Occupied + (SIZE) <= (ALLOCATORPTR)->Capacity)

ScratchAllocator ScratchAllocatorMake(Size size);

Void *ScratchAllocatorAlloc(ScratchAllocator *scratchAllocator, Size size);
void ScratchAllocatorFree(ScratchAllocator *scratchAllocator);

void MemoryCopy(Void *dest, const Void *source, Size size);
void MemorySet(Void *data, Byte value, Size size);
void MemoryZero(Void *data, Size size);

typedef struct Block {
    ScratchAllocator arena;
    struct Block *Next;
} Block;

Block *BlockMake(Size size);

/*
 * Block Allocator
 */
typedef struct {
    Block *Head;
} BlockAllocator;

BlockAllocator BlockAllocatorMake();
BlockAllocator BlockAllocatorMakeEx(Size size);

Void *BlockAllocatorAlloc(BlockAllocator *allocator, Size size);
Void *BlockAllocatorAllocZ(BlockAllocator *allocator, Size size);

void BlockAllocatorFree(BlockAllocator *allocator);

#endif // GFS_MEMORY_H_INCLUDED
