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
GFS_API Vector2I32 GetMousePosition(Window *window);

/*
 * @breaf Sets mouse position.
 * */
GFS_API void SetMousePosition(Window *window, i32 x, i32 y);

typedef enum {
    MOUSEVISIBILITYSTATE_SHOWN,
    MOUSEVISIBILITYSTATE_HIDDEN,
} MouseVisibilityState;

GFS_API void SetMouseVisibility(MouseVisibilityState newState);

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
GFS_API bool IsKeyDown(Key key);

/*
 * @breaf Actual platform-dependend file handle represantation.
 */
typedef struct FileHandle FileHandle;

/*
 * @breaf Checks if platform file handle is valid.
 */
GFS_API bool FileHandleIsValid(FileHandle *handle);

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

GFS_API FileOpenResult FileOpenEx(cstring8 filePath, Scratch *allocator, Permissions permissions);

typedef enum {
    FILE_CLOSE_OK,
    FILE_CLOSE_ERR,
} FileCloseResultCode;

GFS_API FileCloseResultCode FileClose(FileHandle *handle);

typedef enum {
    PLATFORM_FILE_LOAD_OK,
    PLATFORM_FILE_FAILED_TO_READ,
} FileLoadResultCode;

GFS_API FileLoadResultCode FileLoadToBuffer(
    FileHandle *handle, void *buffer, usize numberOfBytesToLoad, usize *numberOfBytesLoaded);

GFS_API FileLoadResultCode FileLoadToBufferEx(
    FileHandle *handle, void *buffer, usize numberOfBytesToLoad, usize *numberOfBytesLoaded, usize loadOffset);

GFS_API usize FileGetSize(FileHandle *handle);

/*
 * @breaf Opens platforms window.
 * */
GFS_API Window *WindowOpen(Scratch *scratch, i32 width, i32 height, cstring8 title);

/*
 * @breaf Closes platform's window.
 * */
GFS_API void WindowClose(Window *window);

/*
 * @breaf Updates window.
 */
GFS_API void WindowUpdate(Window *window);

/*
 * @breaf Resizes render buffer.
 *
 * @warn No implementation
 */
GFS_API void WindowResize(Window *window, i32 width, i32 height);

/*
 * @breaf Returns rectangle of platform's window.
 *
 * TODO(ilya.a): Check X and Y values [2024/08/31]
 */
GFS_API RectangleI32 WindowGetRectangle(Window *window);

/*
 * @breaf Processes platform's events.
 * */
GFS_API void PoolEvents(Window *window);

/*
 * @breaf Returns platform's pagesize.
 * */
GFS_API usize GetPageSize();

/*
 * @breaf Allocates memory aligned to platform's default pagesize.
 * */
GFS_API void *MemoryAllocate(usize size);

typedef enum {
    PLATFORM_MEMORY_FREE_OK,
    PLATFORM_MEMORY_FREE_ERR,
} MemoryFreeResultCode;

/*
 * @breaf Unmaps memory page.
 * */
GFS_API MemoryFreeResultCode MemoryFree(void *data);

/*
 * @breaf Checks if path exists in filesystem.
 */
GFS_API bool IsPathExists(cstring8 path);

/*
 * @breaf Initializes `SoundOutput` struct with default values.
 *
 * TODO(ilya.a): Expose move parameters to the arguments [2024/08/31]
 */
GFS_API SoundOutput SoundOutputMake(i32 samplesPerSecond);

GFS_API void SoundOutputSetTone(SoundOutput *output, i32 toneHZ);

GFS_API SoundDevice *SoundDeviceOpen(Scratch *scratch, Window *window, i32 samplesPerSecond, usize audioBufferSize);

typedef enum {
    PLATFORM_SOUND_DEVICE_GET_CURRENT_POSITION_OK,
    PLATFORM_SOUND_DEVICE_GET_CURRENT_POSITION_ERR,
} SoundDeviceGetCurrentPositionResult;

GFS_API SoundDeviceGetCurrentPositionResult SoundDeviceGetCurrentPosition(
    SoundDevice *device, u32 *playCursor, u32 *writeCursor);

GFS_API void SoundDeviceLockBuffer(
    SoundDevice *device, u32 offset, u32 portionSizeToLock, void **region0, u32 *region0Size, void **region1,
    u32 *region1Size);
GFS_API void SoundDeviceUnlockBuffer(SoundDevice *device, void *region0, u32 region0Size, void *region1, u32 region1Size);
GFS_API void SoundDevicePlay(SoundDevice *device);

GFS_API void SoundDeviceClose(SoundDevice *device);

/*
 * @breaf Puts whole null terminated string to stdout.
 *
 * NOTE: Currently calls `OutputDebugString` for win32 platform.
 * TODO: Propertly implement IO on platform layer.
 */
GFS_API void PutString(cstring8 s);

/*
 * @breaf Called in ASSERT macro in order to print last error of platform level
 * code.
 */
GFS_API void PutLastError(void);

/*
 * @breaf Calls platform specific break function.
 */
GFS_API void ThrowDebugBreak(void);

/*
 * @breaf Exits the process with specified code.
 */
GFS_API void ProcessExit(u32 code);

#endif // GFS_PLATFORM_H_INCLUDED
