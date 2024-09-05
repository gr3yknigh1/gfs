#pragma once
/*
 * FILE      Code\Memory.hpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "Types.hpp"
#include "Macros.hpp"

#define KILOBYTES(X) (1024 * (X))
#define MEGABYTES(X) (1024 * 1024 * (X))
#define GIGABYTES(X) (1024 * 1024 * 1024 * (X))

usize Align2PageSize(usize size);

/*
 * @breaf Scratch Allocator.
 */
typedef struct {
    void *data;
    usize capacity;
    usize occupied;
} ScratchAllocator;

#define SCRATCH_ALLOCATOR_HAS_SPACE(ALLOCATORPTR, SIZE) ((ALLOCATORPTR)->occupied + (SIZE) <= (ALLOCATORPTR)->capacity)

ScratchAllocator ScratchAllocatorMake(usize size);

void *ScratchAllocatorAlloc(ScratchAllocator *scratchAllocator, usize size);
void ScratchAllocatorFree(ScratchAllocator *scratchAllocator);

void MemoryCopy(void *dest, const void *source, usize size);
void MemorySet(void *data, byte value, usize size);
void MemoryZero(void *data, usize size);

typedef struct Block {
    ScratchAllocator arena;
    struct Block *next;
} Block;

Block *BlockMake(usize size);

/*
 * @breaf Block Allocator
 */
typedef struct {
    Block *head;
} BlockAllocator;

BlockAllocator BlockAllocatorMake();
BlockAllocator BlockAllocatorMakeEx(usize size);

void *BlockAllocatorAlloc(BlockAllocator *allocator, usize size);
void *BlockAllocatorAllocZ(BlockAllocator *allocator, usize size);

void BlockAllocatorFree(BlockAllocator *allocator);
