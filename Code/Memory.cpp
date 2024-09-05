/*
 * FILE      Code\Memory.cpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "Memory.hpp"

#include "Platform.hpp"
#include "Types.hpp"
#include "Assert.hpp"
#include "Macros.hpp"

usize
Align2PageSize(usize size) {
    usize pageSize = PlatformGetPageSize();
    return size + (pageSize - size % pageSize);
}

ScratchAllocator
ScratchAllocatorMake(usize size) {
    void *data = PlatformMemoryAllocate(size);
    return LITERAL(ScratchAllocator) {
        .data = data,
        .capacity = size,
        .occupied = 0,
    };
}

void *
ScratchAllocatorAlloc(ScratchAllocator *scratchAllocator, usize size) {
    ASSERT_NONNULL(scratchAllocator);
    ASSERT_NONNULL(scratchAllocator->data);

    if (scratchAllocator->occupied + size > scratchAllocator->capacity) {
        return NULL;
    }

    void *data = ((byte *)scratchAllocator->data) + scratchAllocator->occupied;
    scratchAllocator->occupied += size;
    return data;
}

void
ScratchAllocatorFree(ScratchAllocator *scratchAllocator) {
    ASSERT_NONNULL(scratchAllocator);
    ASSERT_NONNULL(scratchAllocator->data);

    PlatformMemoryFree(scratchAllocator->data);

    scratchAllocator->data = NULL;
    scratchAllocator->capacity = 0;
    scratchAllocator->occupied = 0;
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

    segment = (Block *)allocatedData;
    segment->arena.data = data;
    segment->arena.capacity = bytesAllocated - sizeof(Block);
    segment->arena.occupied = 0;
    segment->next = NULL;

    return segment;
}

BlockAllocator
BlockAllocatorMake() {
    BlockAllocator allocator;
    allocator.head = NULL;
    return allocator;
}

BlockAllocator
BlockAllocatorMakeEx(usize size) {
    BlockAllocator allocator = {0};
    allocator.head = BlockMake(size);
    return allocator;
}

void *
BlockAllocatorAlloc(BlockAllocator *allocator, usize size) {
    ASSERT_NONNULL(allocator);

    Block *previousBlock = NULL;
    Block *currentBlock = allocator->head;

    while (currentBlock != NULL) {
        if (!SCRATCH_ALLOCATOR_HAS_SPACE(&currentBlock->arena, size)) {
            previousBlock = currentBlock;
            currentBlock = currentBlock->next;
            continue;
        }
        return ScratchAllocatorAlloc(&currentBlock->arena, size);
    }

    Block *newBlock = BlockMake(size);

    if (allocator->head == NULL) {
        allocator->head = newBlock;
    } else {
        previousBlock->next = newBlock;
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
    ASSERT_NONNULL(allocator);
    ASSERT_NONNULL(allocator->head);

    Block *previousBlock = NULL;
    Block *currentBlock = allocator->head;

    while (currentBlock != NULL) {
        previousBlock = currentBlock;
        currentBlock = currentBlock->next;

        PlatformMemoryFree(previousBlock);
    }

    allocator->head = NULL;
}
