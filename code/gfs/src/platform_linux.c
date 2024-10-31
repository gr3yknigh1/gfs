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
    FILE *file;
} FileHandle;

bool
FileHandleIsValid(FileHandle *handle)
{
    return handle->file != NULL;
}

FileOpenResult
FileOpenEx(cstring8 filePath, Scratch *allocator, Permissions permissions)
{
    FileOpenResult result = INIT_EMPTY_STRUCT(FileOpenResult);

    result.code = FILE_OPEN_OK;
    result.handle = ScratchAlloc(allocator, sizeof(FileHandle));

    cstring8 openMode = "r";

    if (permissions == PERMISSION_READ) {
        openMode = "r";
    } else if (permissions == PERMISSION_WRITE) {
        openMode = "w";
    } else if (permissions == PERMISSION_READ_WRITE) {
        openMode = "rw";
    }

    result.handle->file = fopen(filePath, openMode);

    if (result.handle->file == NULL) {
        result.handle = NULL;
        result.code = FILE_OPEN_FAILED_TO_OPEN;
    }

    return result;
}

FileCloseResultCode
FileClose(FileHandle *handle)
{
    int result = fclose(handle->file);

    if (result == EOF) {
        return FILE_CLOSE_ERR;
    }

    return FILE_CLOSE_OK;
}

bool
IsPathExists(cstring8 path)
{
    struct stat fileStat = INIT_EMPTY_STRUCT(struct stat);
    return (stat(path, &fileStat) == 0);
}

FileLoadResultCode
FileLoadToBuffer(FileHandle *handle, void *buffer, usize numberOfBytesToLoad, usize *numberOfBytesLoaded)
{
    return FileLoadToBufferEx(handle, buffer, numberOfBytesToLoad, numberOfBytesLoaded, 0);
}

FileLoadResultCode
FileLoadToBufferEx(
    FileHandle *handle, void *buffer, usize numberOfBytesToLoad, usize *numberOfBytesLoaded, usize loadOffset)
{

    long origOffset = 0;

    if (loadOffset != 0) {
        origOffset = ftell(handle->file);
        fseek(handle->file, loadOffset, SEEK_SET);
    }

    size_t bytesReaded = fread(buffer, 1, numberOfBytesToLoad, handle->file);

    if (loadOffset != 0) {
        fseek(handle->file, origOffset, SEEK_SET);
    }

    if (bytesReaded != numberOfBytesToLoad) {
        return FILE_LOAD_ERR;
    }

    if (numberOfBytesLoaded != NULL) {
        *numberOfBytesLoaded = bytesReaded;
    }

    return FILE_LOAD_OK;
}

usize
FileGetSize(FileHandle *handle)
{
    usize size = 0;

    fseek(handle->file, 0, SEEK_END);
    size = ftell(handle->file);
    fseek(handle->file, 0, SEEK_SET);

    return size;
}

void *
MemoryAllocate(usize size)
{
    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    return data;
}

MemoryFreeResultCode
MemoryFree(void *data, usize size)
{
    if (munmap(data, size) != 0) {
        return MEMORY_FREE_ERR;
    }
    return MEMORY_FREE_OK;
}

usize
GetPageSize(void)
{
    usize pageSize = getpagesize();
    return pageSize;
}

void
PutLastError(void)
{
    cstring8 errorMessage = strerror(errno);

    if (errorMessage == NULL) {
        errorMessage = "(null)";
    }

    fprintf(stdout, "E: Linux errno=%s\n", errorMessage);
}

void
PutString(cstring8 s)
{
    puts(s);
}

GFS_NORETURN void
ProcessExit(u32 code)
{
    exit(code);
}

void
ThrowDebugBreak(void)
{
    raise(SIGTRAP);
}
