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

usize Align2PageSize(usize size);

/*
 * @breaf Scratch Allocator.
 */
typedef struct {
    void *Data;
    usize Capacity;
    usize Occupied;
} ScratchAllocator;

#define SCRATCH_ALLOCATOR_HAS_SPACE(ALLOCATORPTR, SIZE) ((ALLOCATORPTR)->Occupied + (SIZE) <= (ALLOCATORPTR)->Capacity)

ScratchAllocator ScratchAllocatorMake(usize size);

void *ScratchAllocatorAlloc(ScratchAllocator *scratchAllocator, usize size);
void ScratchAllocatorFree(ScratchAllocator *scratchAllocator);

void MemoryCopy(void *dest, const void *source, usize size);
void MemorySet(void *data, byte value, usize size);
void MemoryZero(void *data, usize size);

typedef struct Block {
    ScratchAllocator arena;
    struct Block *Next;
} Block;

Block *BlockMake(usize size);

/*
 * @breaf Block Allocator
 */
typedef struct {
    Block *Head;
} BlockAllocator;

BlockAllocator BlockAllocatorMake();
BlockAllocator BlockAllocatorMakeEx(usize size);

void *BlockAllocatorAlloc(BlockAllocator *allocator, usize size);
void *BlockAllocatorAllocZ(BlockAllocator *allocator, usize size);

void BlockAllocatorFree(BlockAllocator *allocator);

#endif // GFS_MEMORY_H_INCLUDED
