/*
 * FILE      gfs_memory.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_memory.h"

#include <Windows.h>

#include "gfs_types.h"
#include "gfs_sys.h"

Size
Align2PageSize(Size size)
{
    Size pageSize = Sys_GetPageSize();
    return size + (pageSize - size % pageSize);
}

Arena
ArenaMake(Size size)
{
    Void *data = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    //                              ^^^^
    // NOTE(ilya.a): So, here I am reserving `size` amount of bytes, but accually `VirtualAlloc`
    // will round up this number to next page. [2024/05/26]
    // TODO(ilya.a): Do something about waste of unused memory in Arena. [2024/05/26]
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

Block *
BlockMake(Size size)
{
    Size bytesAllocated = Align2PageSize(size + sizeof(Block));
    Void *allocatedData = VirtualAlloc(NULL, bytesAllocated, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (allocatedData == NULL)
    {
        return NULL;
    }

    Block *segment = (Block *)allocatedData;
    Void *data = (Byte *)(allocatedData) + sizeof(Block);

    segment = allocatedData;
    segment->arena.Data = data;
    segment->arena.Capacity = bytesAllocated - sizeof(Block);
    segment->arena.Occupied = 0;
    segment->Next = NULL;

    return segment;
}

BlockAllocator
BlockAllocatorMake()
{
    BlockAllocator allocator;
    allocator.Head = NULL;
    return allocator;
}

BlockAllocator 
BlockAllocatorMakeEx(Size size)
{
    BlockAllocator allocator = {0};
    allocator.Head = BlockMake(size);
    return allocator;
}

Void *
BlockAllocatorAlloc(BlockAllocator *allocator, Size size)
{
    if (allocator == NULL)
    {
        return NULL;
    }

    Block *previousBlock = NULL;
    Block *currentBlock = allocator->Head;

    while (currentBlock != NULL)
    {
        if (!ARENA_HAS_SPACE(&currentBlock->arena, size))
        {
            previousBlock = currentBlock;
            currentBlock = currentBlock->Next;
            continue;
        }
        return ArenaAlloc(&currentBlock->arena, size);
    }

    Block *newBlock = BlockMake(size);

    if (allocator->Head == NULL)
    {
        allocator->Head = newBlock;
    }
    else
    {
        previousBlock->Next = newBlock;
    }

    return ArenaAlloc(&newBlock->arena, size);
}

Void *
BlockAllocatorAllocZ(BlockAllocator *allocator, Size size)
{
    Void *data = BlockAllocatorAlloc(allocator, size);
    MemoryZero(data, size);
    return data;
}

void
BlockAllocatorFree(BlockAllocator *allocator)
{
    if (allocator == NULL || allocator->Head == NULL)
    {
        return;
    }

    Block *previousBlock = NULL;
    Block *currentBlock = allocator->Head;

    while (currentBlock != NULL) 
    {
        previousBlock = currentBlock;
        currentBlock = currentBlock->Next;

        VirtualFree(previousBlock, 0, MEM_RELEASE);
    }

    allocator->Head = NULL;
}
