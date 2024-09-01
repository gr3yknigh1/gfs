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

/*
 * @breaf Actual platform-depended window represantation.
 */
typedef struct PlatformWindow PlatformWindow;

/*
 * @breaf Actual platform-dependend sound output device.
 */
typedef struct PlatformSoundDevice PlatformSoundDevice;

/*
 * @breaf Sound output metadata.
 */
typedef struct {
    u32 runningSampleIndex;
    u32 toneHZ;
    i32 samplesPerSecond;
    i32 toneVolume;
    i32 wavePeriod;
    usize bytesPerSample;
    usize audioBufferSize;

    i32 latencySampleCount;
} PlatformSoundOutput;

/*
 * @breaf Actual platform-dependend file handle represantation.
 */
typedef struct PlatformFileHandle PlatformFileHandle;

/*
 * @breaf Checks if platform file handle is valid.
 */
bool PlatformFileHandleIsValid(PlatformFileHandle *handle);

typedef u8 PlatformPermissions;

#define PLATFORM_PERMISSION_READ MKFLAG(1)
#define PLATFORM_PERMISSION_WRITE MKFLAG(2)
#define PLATFORM_PERMISSION_READ_WRITE (PLATFORM_PERMISSION_READ | PLATFORM_PERMISSION_WRITE)

typedef enum {
    PLATFORM_FILE_OPEN_OK,
    PLATFORM_FILE_OPEN_FAILED_TO_OPEN,
} PlatformFileOpenResultCode;

typedef struct {
    PlatformFileHandle *handle;
    PlatformFileOpenResultCode code;
} PlatformFileOpenResult;

// PlatformFileOpenResult PlatformFileOpen(cstring8 filePath);

PlatformFileOpenResult PlatformFileOpenEx(
    cstring8 filePath, ScratchAllocator *allocator, PlatformPermissions permissions);

typedef enum {
    PLATFORM_FILE_LOAD_OK,
    PLATFORM_FILE_FAILED_TO_READ,
} PlatformFileLoadResult;

PlatformFileLoadResult PlatformFileLoadToBuffer(
    PlatformFileHandle *handle, void *buffer, usize numberOfBytesToLoad, usize *numberOfBytesLoaded);
PlatformFileLoadResult PlatformFileLoadToBufferEx(
    PlatformFileHandle *handle, void *buffer, usize numberOfBytesToLoad, usize *numberOfBytesLoaded, usize loadOffset);

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
void PlatformWindowUpdate(
    PlatformWindow *window, i32 windowXOffset, i32 windowYOffset, i32 windowWidth, i32 windowHeight);

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

/*
 * @breaf Initializes `PlatformSoundOutput` struct with default values.
 *
 * TODO(ilya.a): Expose move parameters to the arguments [2024/08/31]
 */
PlatformSoundOutput PlatformSoundOutputMake(i32 samplesPerSecond);

void PlatformSoundOutputSetTone(PlatformSoundOutput *output, i32 toneHZ);

PlatformSoundDevice *PlatformSoundDeviceOpen(
    ScratchAllocator *scratch, PlatformWindow *window, i32 samplesPerSecond, usize audioBufferSize);

typedef enum {
    PLATFORM_SOUND_DEVICE_GET_CURRENT_POSITION_OK,
    PLATFORM_SOUND_DEVICE_GET_CURRENT_POSITION_ERR,
} PlatformSoundDeviceGetCurrentPositionResult;

PlatformSoundDeviceGetCurrentPositionResult PlatformSoundDeviceGetCurrentPosition(
    PlatformSoundDevice *device, u32 *playCursor, u32 *writeCursor);

void PlatformSoundDeviceLockBuffer(
    PlatformSoundDevice *device, u32 offset, u32 portionSizeToLock, void **region0, u32 *region0Size, void **region1,
    u32 *region1Size);
void PlatformSoundDeviceUnlockBuffer(
    PlatformSoundDevice *device, void *region0, u32 region0Size, void *region1, u32 region1Size);
void PlatformSoundDevicePlay(PlatformSoundDevice *device);

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
