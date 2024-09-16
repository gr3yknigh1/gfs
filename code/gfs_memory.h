#if !defined(GFS_MEMORY_H_INCLUDED)
/*
 * FILE      gfs_memory.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#define GFS_MEMORY_H_INCLUDED

#include "gfs_types.h"
#include "gfs_macros.h"

#define KILOBYTES(X) (1024 * (X))
#define MEGABYTES(X) (1024 * 1024 * (X))
#define GIGABYTES(X) (1024 * 1024 * 1024 * (X))

usize Align2PageSize(usize size);

void MemoryCopy(void *dest, const void *source, usize size);
void MemorySet(void *data, byte value, usize size);
void MemoryZero(void *data, usize size);

/*
 * @breaf Stack allocator
 * */
typedef struct {
    void *data;
    usize capacity;
    usize occupied;
} StackAllocator;

/*
 * @breaf Initializes default stack allocator with pre-allocated size.
 *
 * @param size Allocation size.
 * @return New stack allocator.
 * */
StackAllocator StackAllocatorMake(usize size);

void *StackAllocatorAlloc(StackAllocator *allocator, usize size);

void StackAllocatorPop(StackAllocator *allocator);

/*
 * @breaf Scratch Allocator.
 */
typedef struct {
    void *data;
    usize capacity;
    usize occupied;
} Scratch;

/*
 * @breaf Initializes scratch allocator.
 * */
Scratch ScratchMake(usize size);

/*
 * @breaf Bumps internal counter of allocated memory and returns pointer to
 * free chunk.
 *
 * @return Pointer to chuck of memory which user can write on. Overflow of this
 * chuck if UB. Returns `NULL` if capacity can't hold enough.
 * */
void *ScratchAlloc(Scratch *scratch, usize size);

/*
 * @breaf Same as `ScratchAlloc`, but calls `MemoryZero` on allocated block.
 *
 * @return Pointer to chuck of memory which user can write on. Overflow of this
 * chuck if UB. Returns `NULL` if capacity can't hold enough.
 *
 * @see ScratchAlloc
 *
 * */
void *ScratchAllocZero(Scratch *scratch, usize size);

/*
 * @breaf Destroys scratch allocator. Deallocates whole block of memory, which
 * was allocated on `ScratchMake`.
 * */
void ScratchDestroy(Scratch *scratch);

bool ScratchHasSpaceFor(const Scratch *scratch, usize extraSize);

/*
 * @breaf Block for block-allocator. Contains arena and pointer to
 * next block in allocator.
 * */
typedef struct Block {
    Scratch arena;
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

#endif // GFS_MEMORY_H_INCLUDED
