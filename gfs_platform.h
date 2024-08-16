#if !defined(GFS_PLATFORM_H_INCLUDED)
/*
 * FILE      gfs_platform.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_PLATFORM_H_INCLUDED

#include "gfs_types.h"
#include "gfs_macros.h"
#include "gfs_memory.h"

typedef struct PlatformWindow PlatformWindow;
typedef struct PlatformSoundDevice PlatformSoundDevice;

/*
 * @breaf Opens platforms window.
 * */
PlatformWindow *PlatformWindowOpen(ScratchAllocator *scratch, i32 width, i32 height, cstr8 title);

/*
 * @breaf Closes platform's window.
 * */
void PlatformWindowClose(PlatformWindow *window);

/*
 * @breaf Processes platform's events.
 * */
void PlatformPoolEvents(PlatformWindow *window);

/*
 * @breaf Returns platform's pagesize.
 * */
usize PlatformGetPageSize();

/*
 * @breaf Allocates memory aligned to platform's default pagesize.
 * */
void *PlatformMemoryAllocate(usize size);

/*
 * @breaf Unmaps memory page.
 * */
void PlatformMemoryFree(void *ptr);


PlatformSoundDevice *PlatformSoundDeviceOpen();
void PlatformSoundDeviceClose(PlatformSoundDevice *device);

#endif // GFS_PLATFORM_H_INCLUDED
