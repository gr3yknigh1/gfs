/*
 * FILE      code/gfs/src/platform_linux.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs/platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "gfs/types.h"
#include "gfs/macros.h"

typedef struct FileHandle {
    int descriptor;
} FileHandle;

bool
FileHandleIsValid(FileHandle *handle) {
    // NOTE(gr3yknigh1): Not properly tested. [2024/10/21]
    bool result = fcntl(handle->descriptor, F_GETFD) != -1;
    return result;
}

FileOpenResult
FileOpenEx(cstring8 filePath, Scratch *allocator, Permissions permissions) {
    FileOpenResult result = INIT_EMPTY_STRUCT(FileOpenResult);

    result.code = FILE_OPEN_OK;
    result.handle = ScratchAlloc(allocator, sizeof(FileHandle));

    int openFlags = 0;

    if (HASANYBIT(permissions, PERMISSION_READ)) {
        openFlags |= O_RDONLY;
    }

    if (HASANYBIT(permissions, PERMISSION_WRITE)) {
        openFlags |= O_WRONLY;
    }

    result.handle->descriptor = open(filePath, openFlags);

    return result;
}

FileCloseResultCode
FileClose(FileHandle *handle) {
    int result = close(handle->descriptor);

    if (result == -1) {
        return FILE_CLOSE_ERR;
    }

    return FILE_CLOSE_OK;
}

bool
IsPathExists(cstring8 path) {
    struct stat fileStat = INIT_EMPTY_STRUCT(struct stat);
    return (stat(path, &fileStat) == 0);
}

FileLoadResultCode
FileLoadToBuffer(FileHandle *handle, void *buffer, usize numberOfBytesToLoad, usize *numberOfBytesLoaded) {
    return FileLoadToBufferEx(handle, buffer, numberOfBytesToLoad, numberOfBytesLoaded, 0);
}

FileLoadResultCode
FileLoadToBufferEx(
    FileHandle *handle, void *buffer, usize numberOfBytesToLoad, usize *numberOfBytesLoaded, usize loadOffset) {

    off_t origOffset = lseek(handle->descriptor, 0, SEEK_CUR);
    ssize_t bytesReaded = read(handle->descriptor, buffer, numberOfBytesToLoad);
    lseek(handle->descriptor, origOffset, SEEK_SET);

    if (bytesReaded == -1) {
        return FILE_LOAD_ERR;
    }

    if (numberOfBytesLoaded != NULL) {
        *numberOfBytesLoaded = bytesReaded;
    }

    return FILE_LOAD_OK;
}

usize
FileGetSize(FileHandle *handle) {
    usize size = 0;

    lseek(handle->descriptor, 0, SEEK_END);
    size = lseek(handle->descriptor, 0, SEEK_CUR);
    lseek(handle->descriptor, 0, SEEK_SET);

    return size;
}

void *
MemoryAllocate(usize size) {
    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    return data;
}

MemoryFreeResultCode
MemoryFree(void *data, usize size) {
    if (munmap(data, size) != 0) {
        return MEMORY_FREE_ERR;
    }
    return MEMORY_FREE_OK;
}

usize
GetPageSize(void) {
    usize pageSize = getpagesize();
    return pageSize;
}

void
PutLastError(void) {
    cstring8 errorMessage = strerror(errno);

    if (errorMessage == NULL) {
        errorMessage = "(null)";
    }

    fprintf(stdout, "E: Linux errno=%s\n", errorMessage);
}

void
PutString(cstring8 s) {
    puts(s);
}

GFS_NORETURN void
ProcessExit(u32 code) {
    exit(code);
}

void
ThrowDebugBreak(void) {
    raise(SIGTRAP);
}
