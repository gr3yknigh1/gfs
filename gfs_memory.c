/*
 * FILE      gfs_memory.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_memory.h"

#include <Windows.h>

#include "gfs_platform.h"
#include "gfs_types.h"
#include "gfs_sys.h"

usize
Align2PageSize(usize size) {
    usize pageSize = Sys_GetPageSize();
    return size + (pageSize - size % pageSize);
}

ScratchAllocator
ScratchAllocatorMake(usize size) {
    void *data = PlatformMemoryAllocate(size);
    return (ScratchAllocator){
        .Data = data,
        .Capacity = size,
        .Occupied = 0,
    };
}

void *
ScratchAllocatorAlloc(ScratchAllocator *scratchAllocator, usize size) {
    if (scratchAllocator == NULL || scratchAllocator->Data == NULL) {
        return NULL;
    }

    if (scratchAllocator->Occupied + size > scratchAllocator->Capacity) {
        return NULL;
    }

    void *data = ((byte *)scratchAllocator->Data) + scratchAllocator->Occupied;
    scratchAllocator->Occupied += size;
    return data;
}

void
ScratchAllocatorFree(ScratchAllocator *scratchAllocator) {
    if (scratchAllocator == NULL || scratchAllocator->Data == NULL) {
        return;
    }

    PlatformMemoryFree(scratchAllocator->Data);

    scratchAllocator->Data = NULL;
    scratchAllocator->Capacity = 0;
    scratchAllocator->Occupied = 0;
}

// TODO(ilya.a): Use SIMD [2024/05/19]

void
MemoryCopy(void *destination, const void *source, usize size) {
    for (usize i = 0; i < size; ++i) {
        ((byte *)destination)[i] = ((byte *)source)[i];
    }
}

void
MemorySet(void *data, byte value, usize size) {
    if (size % 4 == 0) {
        for (usize i = 0; i < size / 4; i += 4) {
            ((byte *)data)[i] = value;
            ((byte *)data)[i + 1] = value;
            ((byte *)data)[i + 2] = value;
            ((byte *)data)[i + 3] = value;
        }
    } else {
        for (usize i = 0; i < size; ++i) {
            ((byte *)data)[i] = value;
        }
    }
}

void
MemoryZero(void *data, usize size) {
    for (usize i = 0; i < size; ++i) {
        ((byte *)data)[i] = 0;
    }
}

Block *
BlockMake(usize size) {
    usize bytesAllocated = Align2PageSize(size + sizeof(Block));
    void *allocatedData = PlatformMemoryAllocate(bytesAllocated);

    if (allocatedData == NULL) {
        return NULL;
    }

    Block *segment = (Block *)allocatedData;
    void *data = (byte *)(allocatedData) + sizeof(Block);

    segment = allocatedData;
    segment->arena.Data = data;
    segment->arena.Capacity = bytesAllocated - sizeof(Block);
    segment->arena.Occupied = 0;
    segment->Next = NULL;

    return segment;
}

BlockAllocator
BlockAllocatorMake() {
    BlockAllocator allocator;
    allocator.Head = NULL;
    return allocator;
}

BlockAllocator
BlockAllocatorMakeEx(usize size) {
    BlockAllocator allocator = {0};
    allocator.Head = BlockMake(size);
    return allocator;
}

void *
BlockAllocatorAlloc(BlockAllocator *allocator, usize size) {
    if (allocator == NULL) {
        return NULL;
    }

    Block *previousBlock = NULL;
    Block *currentBlock = allocator->Head;

    while (currentBlock != NULL) {
        if (!SCRATCH_ALLOCATOR_HAS_SPACE(&currentBlock->arena, size)) {
            previousBlock = currentBlock;
            currentBlock = currentBlock->Next;
            continue;
        }
        return ScratchAllocatorAlloc(&currentBlock->arena, size);
    }

    Block *newBlock = BlockMake(size);

    if (allocator->Head == NULL) {
        allocator->Head = newBlock;
    } else {
        previousBlock->Next = newBlock;
    }

    return ScratchAllocatorAlloc(&newBlock->arena, size);
}

void *
BlockAllocatorAllocZ(BlockAllocator *allocator, usize size) {
    void *data = BlockAllocatorAlloc(allocator, size);
    MemoryZero(data, size);
    return data;
}

void
BlockAllocatorFree(BlockAllocator *allocator) {
    if (allocator == NULL || allocator->Head == NULL) {
        return;
    }

    Block *previousBlock = NULL;
    Block *currentBlock = allocator->Head;

    while (currentBlock != NULL) {
        previousBlock = currentBlock;
        currentBlock = currentBlock->Next;

        PlatformMemoryFree(previousBlock);
    }

    allocator->Head = NULL;
}
