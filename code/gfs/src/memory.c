/*
 * FILE      gfs_memory.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs/memory.h"

#include "gfs/platform.h"
#include "gfs/types.h"
#include "gfs/assert.h"

usize
Align2PageSize(usize size) {
    usize pageSize = GetPageSize();
    return size + (pageSize - size % pageSize);
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

typedef struct {
    usize size;
} AllocationInfo;

EXPECT_TYPE_SIZE(AllocationInfo, 8);

StackAllocator
StackAllocatorMake(usize size) {
    StackAllocator allocator = {0};

    allocator.data = MemoryAllocate(size);
    allocator.capacity = size;
    allocator.occupied = 0;

    return allocator;
}

void *
StackAllocatorAlloc(StackAllocator *allocator, usize size) {
    ASSERT_NONNULL(allocator);
    ASSERT_NONNULL(allocator->data);

    usize needsToBeAllocated = sizeof(AllocationInfo) + size;

    if (allocator->occupied + needsToBeAllocated > allocator->capacity) {
        return NULL;
    }

    byte *data = ((byte *)allocator->data) + allocator->occupied;
    allocator->occupied += needsToBeAllocated;

    /*
     * Writes `AllocationInfo` AFTER client's code data. Because of
     * strategy in `StackAllocatorPop`: we searching for metadata
     * at the end of allocation block
     * */
    AllocationInfo *info = (AllocationInfo *)(data + size);
    info->size = size;
    data += sizeof(AllocationInfo);

    return data;
}

void
StackAllocatorPop(StackAllocator *allocator) {
    ASSERT_NONNULL(allocator);
    ASSERT_NONNULL(allocator->data);

    void *data = ((byte *)allocator->data) + allocator->occupied;
    UNUSED(data);
}

Scratch
ScratchMake(usize size) {
    Scratch ret = {0};

    ret.data = MemoryAllocate(size);
    ret.capacity = size;
    ret.occupied = 0;

    return ret;
}

void *
ScratchAlloc(Scratch *scratch, usize size) {
    ASSERT_NONNULL(scratch);
    ASSERT_NONNULL(scratch->data);

    if (scratch->occupied + size > scratch->capacity) {
        return NULL;
    }

    void *data = ((byte *)scratch->data) + scratch->occupied;
    scratch->occupied += size;
    return data;
}

void *
ScratchAllocZero(Scratch *scratch, usize size) {
    void *data = ScratchAlloc(scratch, size);

    if (data != NULL) {
        MemoryZero(data, size);
    }

    return data;
}

void
ScratchDestroy(Scratch *scratch) {
    ASSERT_NONNULL(scratch);
    ASSERT_NONNULL(scratch->data);

    MemoryFree(scratch->data, scratch->capacity);

    scratch->data = NULL;
    scratch->capacity = 0;
    scratch->occupied = 0;
}

bool
ScratchHasSpaceFor(const Scratch *scratch, usize extraSize) {
    return scratch->occupied + extraSize <= scratch->capacity;
}

AllocationBlock *
BlockMake(usize size) {
    usize bytesAllocated = Align2PageSize(size + sizeof(AllocationBlock));
    void *allocatedData = MemoryAllocate(bytesAllocated);

    if (allocatedData == NULL) {
        return NULL;
    }

    AllocationBlock *segment = (AllocationBlock *)allocatedData;
    void *data = (byte *)(allocatedData) + sizeof(AllocationBlock);

    segment = allocatedData;
    segment->arena.data = data;
    segment->arena.capacity = bytesAllocated - sizeof(AllocationBlock);
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

    AllocationBlock *previousBlock = NULL;
    AllocationBlock *currentBlock = allocator->head;

    while (currentBlock != NULL) {
        if (!ScratchHasSpaceFor(&currentBlock->arena, size)) {
            previousBlock = currentBlock;
            currentBlock = currentBlock->next;
            continue;
        }
        return ScratchAlloc(&currentBlock->arena, size);
    }

    AllocationBlock *newBlock = BlockMake(size);

    if (allocator->head == NULL) {
        allocator->head = newBlock;
    } else {
        previousBlock->next = newBlock;
    }

    return ScratchAlloc(&newBlock->arena, size);
}

void *
BlockAllocatorAllocZ(BlockAllocator *allocator, usize size) {
    void *data = BlockAllocatorAlloc(allocator, size);
    MemoryZero(data, size);
    return data;
}

void
BlockAllocatorDestroy(BlockAllocator *allocator) {
    ASSERT_NONNULL(allocator);
    ASSERT_NONNULL(allocator->head);

    AllocationBlock *previousBlock = NULL;
    AllocationBlock *currentBlock = allocator->head;

    while (currentBlock != NULL) {
        previousBlock = currentBlock;
        currentBlock = currentBlock->next;

        MemoryFree(previousBlock, previousBlock->arena.capacity + sizeof(AllocationBlock));
        //                        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        // NOTE(gr3yknigh1): This not properly tested [2024/10/21]
    }

    allocator->head = NULL;
}


Scratch
TempScratchMake(Scratch *scratch, usize capacity) {
    Scratch tempScratch = INIT_EMPTY_STRUCT(Scratch);
    tempScratch.data = ScratchAlloc(scratch, capacity);
    tempScratch.capacity = capacity;
    tempScratch.occupied = 0;
    return tempScratch;
}

void
TempScratchClean(Scratch *temp, Scratch *scratch) {
    ASSERT_EQ(temp->data, ((byte *)scratch->data + scratch->occupied) - temp->capacity);
    scratch->occupied -= temp->capacity;

    temp->data = NULL;
    temp->capacity = 0;
    temp->occupied = 0;
}
