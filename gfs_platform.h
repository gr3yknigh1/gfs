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
#include "gfs_physics.h"

typedef struct PlatformWindow PlatformWindow;
typedef struct PlatformSoundDevice PlatformSoundDevice;

/*
 * @breaf Opens platforms window.
 * */
PlatformWindow *PlatformWindowOpen(ScratchAllocator *scratch, i32 width, i32 height, cstring8 title);

/*
 * @breaf Closes platform's window.
 * */
void PlatformWindowClose(PlatformWindow *window);

/*
 * @breaf Blits render buffer to PlatformWindow.
 */
void PlatformWindowUpdate(PlatformWindow *window, i32 windowXOffset, i32 windowYOffset, i32 windowWidth, i32 windowHeight);

/*
 * @breaf Resizes render buffer.
 */
void PlatformWindowResize(PlatformWindow *window, i32 width, i32 height);

/*
 * @breaf Returns rectangle of platform's window.
 *
 * TODO(ilya.a): Check X and Y values [2024/08/31]
 */
RectangleI32 PlatformWindowGetRectangle(PlatformWindow *window);

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

typedef enum {
    PLATFORM_MEMORY_FREE_OK,
    PLATFORM_MEMORY_FREE_ERR,
} PlatformMemoryFreeResult;

/*
 * @breaf Unmaps memory page.
 * */
PlatformMemoryFreeResult PlatformMemoryFree(void *data);

/*
 * @breaf Checks if path exists in filesystem.
 */
bool PlatformIsPathExists(cstring8 path);

PlatformSoundDevice *PlatformSoundDeviceOpen(ScratchAllocator *scratch, PlatformWindow *window);

void PlatformSoundDeviceClose(PlatformSoundDevice *device);

/*
 * @breaf Puts whole null terminated string to stdout.
 *
 * NOTE: Currently calls `OutputDebugString` for win32 platform.
 * TODO: Propertly implement IO on platform layer.
 */
void PlatformPutString(cstring8 s);

/*
 * @breaf Calls platform specific break function.
 */
void PlatformDebugBreak(void);

/*
 * @breaf Exits the process with specified code.
 */
void PlatformExitProcess(u32 code);

#endif // GFS_PLATFORM_H_INCLUDED
