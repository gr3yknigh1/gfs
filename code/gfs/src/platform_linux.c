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

#include "gfs/types.h"
#include "gfs/macros.h"

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
