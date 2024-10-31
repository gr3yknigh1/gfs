#if !defined(GFS_MEMORY_H_INCLUDED)
/*
 * FILE      gfs\code\gfs\include\memory.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#define GFS_MEMORY_H_INCLUDED

#include "gfs/types.h"
#include "gfs/macros.h"

#define KILOBYTES(X) (1024 * (X))
#define MEGABYTES(X) (1024 * 1024 * (X))
#define GIGABYTES(X) (1024 * 1024 * 1024 * (X))

GFS_API usize Align2PageSize(usize size);

GFS_API void MemoryCopy(void *dest, const void *source, usize size);
GFS_API void MemorySet(void *data, byte value, usize size);
GFS_API void MemoryZero(void *data, usize size);

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
GFS_API StackAllocator StackAllocatorMake(usize size);

GFS_API void *StackAllocatorAlloc(StackAllocator *allocator, usize size);

GFS_API void StackAllocatorPop(StackAllocator *allocator);

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
GFS_API Scratch ScratchMake(usize size);

GFS_API Scratch TempScratchMake(Scratch *scratch, usize capacity);

GFS_API void TempScratchClean(Scratch *temp, Scratch *scratch);

/*
 * @breaf Bumps internal counter of allocated memory and returns pointer to
 * free chunk.
 *
 * @return Pointer to chuck of memory which user can write on. Overflow of this
 * chuck if UB. Returns `NULL` if capacity can't hold enough.
 * */
GFS_API void *ScratchAlloc(Scratch *scratch, usize size);

/*
 * @breaf Same as `ScratchAlloc`, but calls `MemoryZero` on allocated block.
 *
 * @return Pointer to chuck of memory which user can write on. Overflow of this
 * chuck if UB. Returns `NULL` if capacity can't hold enough.
 *
 * @see ScratchAlloc
 *
 * */
GFS_API void *ScratchAllocZero(Scratch *scratch, usize size);

/*
 * @breaf Destroys scratch allocator. Deallocates whole block of memory, which
 * was allocated on `ScratchMake`.
 * */
GFS_API void ScratchDestroy(Scratch *scratch);

GFS_API bool ScratchHasSpaceFor(const Scratch *scratch, usize extraSize);

/*
 * @breaf Block for block-allocator. Contains arena and pointer to
 * next block in allocator.
 * */
typedef struct AllocationBlock {
    Scratch arena;
    struct AllocationBlock *next;
} AllocationBlock;

AllocationBlock *BlockMake(usize size);

/*
 * @breaf Block Allocator
 */
typedef struct {
    AllocationBlock *head;
} BlockAllocator;

GFS_API BlockAllocator BlockAllocatorMake();
GFS_API BlockAllocator BlockAllocatorMakeEx(usize size);

GFS_API void *BlockAllocatorAlloc(BlockAllocator *allocator, usize size);
GFS_API void *BlockAllocatorAllocZ(BlockAllocator *allocator, usize size);

GFS_API void BlockAllocatorDestroy(BlockAllocator *allocator);

#endif // GFS_MEMORY_H_INCLUDED
