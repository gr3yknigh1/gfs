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
typedef struct Window Window;

/*
 * @breaf Actual platform-dependend sound output device.
 */
typedef struct SoundDevice SoundDevice;

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
} SoundOutput;

/*
 * @breaf Actual platform-dependend file handle represantation.
 */
typedef struct FileHandle FileHandle;

/*
 * @breaf Checks if platform file handle is valid.
 */
bool FileHandleIsValid(FileHandle *handle);

typedef u8 Permissions;

#define PLATFORM_PERMISSION_READ MKFLAG(1)
#define PLATFORM_PERMISSION_WRITE MKFLAG(2)
#define PLATFORM_PERMISSION_READ_WRITE                                         \
    (PLATFORM_PERMISSION_READ | PLATFORM_PERMISSION_WRITE)

typedef enum {
    PLATFORM_FILE_OPEN_OK,
    PLATFORM_FILE_OPEN_FAILED_TO_OPEN,
} FileOpenResultCode;

typedef struct {
    FileHandle *handle;
    FileOpenResultCode code;
} FileOpenResult;

// FileOpenResult FileOpen(cstring8 filePath);

FileOpenResult FileOpenEx(
    cstring8 filePath, Scratch *allocator, Permissions permissions);

typedef enum {
    FILE_CLOSE_OK,
    FILE_CLOSE_ERR,
} FileCloseResultCode;

FileCloseResultCode FileClose(FileHandle *handle);

typedef enum {
    PLATFORM_FILE_LOAD_OK,
    PLATFORM_FILE_FAILED_TO_READ,
} FileLoadResultCode;

FileLoadResultCode FileLoadToBuffer(
    FileHandle *handle, void *buffer, usize numberOfBytesToLoad,
    usize *numberOfBytesLoaded);
FileLoadResultCode FileLoadToBufferEx(
    FileHandle *handle, void *buffer, usize numberOfBytesToLoad,
    usize *numberOfBytesLoaded, usize loadOffset);

usize FileGetSize(FileHandle *handle);

/*
 * @breaf Opens platforms window.
 * */
Window *WindowOpen(Scratch *scratch, i32 width, i32 height, cstring8 title);

/*
 * @breaf Closes platform's window.
 * */
void WindowClose(Window *window);

/*
 * @breaf Updates window.
 */
void WindowUpdate(Window *window);

/*
 * @breaf Resizes render buffer.
 */
void WindowResize(Window *window, i32 width, i32 height);

/*
 * @breaf Returns rectangle of platform's window.
 *
 * TODO(ilya.a): Check X and Y values [2024/08/31]
 */
RectangleI32 WindowGetRectangle(Window *window);

/*
 * @breaf Processes platform's events.
 * */
void PoolEvents(Window *window);

/*
 * @breaf Returns platform's pagesize.
 * */
usize GetPageSize();

/*
 * @breaf Allocates memory aligned to platform's default pagesize.
 * */
void *MemoryAllocate(usize size);

typedef enum {
    PLATFORM_MEMORY_FREE_OK,
    PLATFORM_MEMORY_FREE_ERR,
} MemoryFreeResultCode;

/*
 * @breaf Unmaps memory page.
 * */
MemoryFreeResultCode MemoryFree(void *data);

/*
 * @breaf Checks if path exists in filesystem.
 */
bool IsPathExists(cstring8 path);

/*
 * @breaf Initializes `SoundOutput` struct with default values.
 *
 * TODO(ilya.a): Expose move parameters to the arguments [2024/08/31]
 */
SoundOutput SoundOutputMake(i32 samplesPerSecond);

void SoundOutputSetTone(SoundOutput *output, i32 toneHZ);

SoundDevice *SoundDeviceOpen(
    Scratch *scratch, Window *window, i32 samplesPerSecond,
    usize audioBufferSize);

typedef enum {
    PLATFORM_SOUND_DEVICE_GET_CURRENT_POSITION_OK,
    PLATFORM_SOUND_DEVICE_GET_CURRENT_POSITION_ERR,
} SoundDeviceGetCurrentPositionResult;

SoundDeviceGetCurrentPositionResult SoundDeviceGetCurrentPosition(
    SoundDevice *device, u32 *playCursor, u32 *writeCursor);

void SoundDeviceLockBuffer(
    SoundDevice *device, u32 offset, u32 portionSizeToLock, void **region0,
    u32 *region0Size, void **region1, u32 *region1Size);
void SoundDeviceUnlockBuffer(
    SoundDevice *device, void *region0, u32 region0Size, void *region1,
    u32 region1Size);
void SoundDevicePlay(SoundDevice *device);

void SoundDeviceClose(SoundDevice *device);

/*
 * @breaf Puts whole null terminated string to stdout.
 *
 * NOTE: Currently calls `OutputDebugString` for win32 platform.
 * TODO: Propertly implement IO on platform layer.
 */
void PutString(cstring8 s);

/*
 * @breaf Called in ASSERT macro in order to print last error of platform level
 * code.
 */
void PutLastError(void);

/*
 * @breaf Calls platform specific break function.
 */
void ThrowDebugBreak(void);

/*
 * @breaf Exits the process with specified code.
 */
void ProcessExit(u32 code);

#endif // GFS_PLATFORM_H_INCLUDED
