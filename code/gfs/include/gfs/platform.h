#if !defined(GFS_PLATFORM_H_INCLUDED)
/*
 * FILE      gfs_platform.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_PLATFORM_H_INCLUDED

#include "gfs/types.h"
#include "gfs/macros.h"
#include "gfs/memory.h"
#include "gfs/physics.h"

/*
 * @breaf Actual platform-depended window represantation.
 */
typedef struct Window Window;

/*
 * @breaf Actual platform-dependend sound output device.
 */
typedef struct SoundDevice SoundDevice;

/*
 * @breaf Sound output data.
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
 * @breaf Returns mouse's position.
 * */
GFS_EXPORT Vector2I32 GetMousePosition(Window *window);

/*
 * @breaf Sets mouse position.
 * */
GFS_EXPORT void SetMousePosition(Window *window, i32 x, i32 y);

typedef enum {
    MOUSEVISIBILITYSTATE_SHOWN,
    MOUSEVISIBILITYSTATE_HIDDEN,
} MouseVisibilityState;

GFS_EXPORT void SetMouseVisibility(MouseVisibilityState newState);

/*
 * @breaf IDs of keyboard keys.
 *
 * TODO: Add more keys.
 * */
typedef enum {
    KEY_NONE,
    KEY_W,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_COUNT,
} Key;

#define KEY_STATE_UP MKFLAG(0)
#define KEY_STATE_DOWN MKFLAG(1)

/*
 * @breaf Returns true, if key is currently pressed by user.
 *
 * Only when he focused on window.
 * */
GFS_EXPORT bool IsKeyDown(Key key);

/*
 * @breaf Actual platform-dependend file handle represantation.
 */
typedef struct FileHandle FileHandle;

/*
 * @breaf Checks if platform file handle is valid.
 */
GFS_EXPORT bool FileHandleIsValid(FileHandle *handle);

typedef u8 Permissions;

#define PLATFORM_PERMISSION_READ MKFLAG(1)
#define PLATFORM_PERMISSION_WRITE MKFLAG(2)
#define PLATFORM_PERMISSION_READ_WRITE (PLATFORM_PERMISSION_READ | PLATFORM_PERMISSION_WRITE)

typedef enum {
    PLATFORM_FILE_OPEN_OK,
    PLATFORM_FILE_OPEN_FAILED_TO_OPEN,
} FileOpenResultCode;

typedef struct {
    FileHandle *handle;
    FileOpenResultCode code;
} FileOpenResult;

// FileOpenResult FileOpen(cstring8 filePath);

GFS_EXPORT FileOpenResult FileOpenEx(cstring8 filePath, Scratch *allocator, Permissions permissions);

typedef enum {
    FILE_CLOSE_OK,
    FILE_CLOSE_ERR,
} FileCloseResultCode;

GFS_EXPORT FileCloseResultCode FileClose(FileHandle *handle);

typedef enum {
    PLATFORM_FILE_LOAD_OK,
    PLATFORM_FILE_FAILED_TO_READ,
} FileLoadResultCode;

GFS_EXPORT FileLoadResultCode FileLoadToBuffer(
    FileHandle *handle, void *buffer, usize numberOfBytesToLoad, usize *numberOfBytesLoaded);

GFS_EXPORT FileLoadResultCode FileLoadToBufferEx(
    FileHandle *handle, void *buffer, usize numberOfBytesToLoad, usize *numberOfBytesLoaded, usize loadOffset);

GFS_EXPORT usize FileGetSize(FileHandle *handle);

/*
 * @breaf Opens platforms window.
 * */
GFS_EXPORT Window *WindowOpen(Scratch *scratch, i32 width, i32 height, cstring8 title);

/*
 * @breaf Closes platform's window.
 * */
GFS_EXPORT void WindowClose(Window *window);

/*
 * @breaf Updates window.
 */
GFS_EXPORT void WindowUpdate(Window *window);

/*
 * @breaf Resizes render buffer.
 *
 * @warn No implementation
 */
GFS_EXPORT void WindowResize(Window *window, i32 width, i32 height);

/*
 * @breaf Returns rectangle of platform's window.
 *
 * TODO(ilya.a): Check X and Y values [2024/08/31]
 */
GFS_EXPORT RectangleI32 WindowGetRectangle(Window *window);

/*
 * @breaf Processes platform's events.
 * */
GFS_EXPORT void PoolEvents(Window *window);

/*
 * @breaf Returns platform's pagesize.
 * */
GFS_EXPORT usize GetPageSize();

/*
 * @breaf Allocates memory aligned to platform's default pagesize.
 * */
GFS_EXPORT void *MemoryAllocate(usize size);

typedef enum {
    PLATFORM_MEMORY_FREE_OK,
    PLATFORM_MEMORY_FREE_ERR,
} MemoryFreeResultCode;

/*
 * @breaf Unmaps memory page.
 * */
GFS_EXPORT MemoryFreeResultCode MemoryFree(void *data);

/*
 * @breaf Checks if path exists in filesystem.
 */
GFS_EXPORT bool IsPathExists(cstring8 path);

/*
 * @breaf Initializes `SoundOutput` struct with default values.
 *
 * TODO(ilya.a): Expose move parameters to the arguments [2024/08/31]
 */
GFS_EXPORT SoundOutput SoundOutputMake(i32 samplesPerSecond);

GFS_EXPORT void SoundOutputSetTone(SoundOutput *output, i32 toneHZ);

GFS_EXPORT SoundDevice *SoundDeviceOpen(Scratch *scratch, Window *window, i32 samplesPerSecond, usize audioBufferSize);

typedef enum {
    PLATFORM_SOUND_DEVICE_GET_CURRENT_POSITION_OK,
    PLATFORM_SOUND_DEVICE_GET_CURRENT_POSITION_ERR,
} SoundDeviceGetCurrentPositionResult;

GFS_EXPORT SoundDeviceGetCurrentPositionResult SoundDeviceGetCurrentPosition(
    SoundDevice *device, u32 *playCursor, u32 *writeCursor);

GFS_EXPORT void SoundDeviceLockBuffer(
    SoundDevice *device, u32 offset, u32 portionSizeToLock, void **region0, u32 *region0Size, void **region1,
    u32 *region1Size);
GFS_EXPORT void SoundDeviceUnlockBuffer(SoundDevice *device, void *region0, u32 region0Size, void *region1, u32 region1Size);
GFS_EXPORT void SoundDevicePlay(SoundDevice *device);

GFS_EXPORT void SoundDeviceClose(SoundDevice *device);

/*
 * @breaf Puts whole null terminated string to stdout.
 *
 * NOTE: Currently calls `OutputDebugString` for win32 platform.
 * TODO: Propertly implement IO on platform layer.
 */
GFS_EXPORT void PutString(cstring8 s);

/*
 * @breaf Called in ASSERT macro in order to print last error of platform level
 * code.
 */
GFS_EXPORT void PutLastError(void);

/*
 * @breaf Calls platform specific break function.
 */
GFS_EXPORT void ThrowDebugBreak(void);

/*
 * @breaf Exits the process with specified code.
 */
GFS_EXPORT void ProcessExit(u32 code);

#endif // GFS_PLATFORM_H_INCLUDED
