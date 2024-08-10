/*
 * FILE      gfs_io.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_io.h"

#include <Windows.h>

#include "gfs_macros.h"
#include "gfs_string.h"

IOResult
IOOpenFile(CStr8 filePath, FileHandle *handleOut, IOPermissions perms) {
    if (filePath == NULL || handleOut == NULL || CStr8IsEmpty(filePath)) {
        return IO_ERR_INVALID_ARGS;
    }

    DWORD desiredAccess = 0;

    if (HASANYBIT(perms, IO_READ)) {
        desiredAccess |= GENERIC_READ;
    }

    if (HASANYBIT(perms, IO_WRITE)) {
        desiredAccess |= GENERIC_WRITE;
    }

    // TODO(ilya.a): Expose sharing options [2024/05/26]
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
    DWORD shareMode = FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE;

    HANDLE handle = CreateFileA(filePath, desiredAccess, shareMode, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        return IO_ERR_FAILED_TO_OPEN;
    }

    *handleOut = handle;

    return IO_OK;
}

// TODO(ilya.a): Should I inline this? [2024/05/26]
IOResult
IOLoadBytesFromFile(FileHandle *handle, Void *buffer, Size numberOfBytes) {
    return IOLoadBytesFromFileEx(handle, buffer, numberOfBytes, 0);
}

IOResult
IOLoadBytesFromFileEx(FileHandle *handle, Void *buffer, Size numberOfBytes, Size offset) {
    if (handle == NULL || buffer == NULL || IO_HANDLE_IS_VALID(*handle)) {
        return IO_ERR_INVALID_ARGS;
    }

    if (numberOfBytes == 0) {
        return IO_OK;
    }

#if 0
    // TODO(ilya.a): Check what was wrong about this piece of code.
    // (Unable to offset and read properly). [2024/05/26]

    LARGE_INTEGER offsetPair;
    offsetPair.LowPart = ((U32 *)&offset)[1];
    offsetPair.HighPart = ((U32 *)&offset)[0];
#endif

    DWORD setFilePointerResult = SetFilePointer(
        *handle,
        (U32)offset, // XXX(ilya.a): Hack [2024/05/26]
        NULL, FILE_BEGIN);

    if (setFilePointerResult == INVALID_SET_FILE_POINTER) {
        return IO_ERR_FAILED_TO_OFFSET_HANDLE;
    }

    DWORD numberOfBytesRead = 0;

    BOOL readFileResult = ReadFile(*handle, buffer, numberOfBytes, &numberOfBytesRead, NULL);

    if (readFileResult != TRUE) {
        return IO_ERR_FAILED_TO_READ;
    }

    if (numberOfBytesRead != numberOfBytes) {
        return IO_ERR_READED_LESS_THAN_REQUIRED;
    }

    return IO_OK;
}
