/*
 * FILE      gfs_io.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_IO_H_INCLUDED
#define GFS_IO_H_INCLUDED

#include "gfs_types.h"
#include "gfs_string.h"
#include "gfs_macros.h"

#include <Windows.h>

// TODO(ilya.a): If at some day I will port it to another platform, do work around with
// windows handles and unix file descriptors [2024/05/25] #win32
typedef HANDLE FileHandle;

#define IO_HANDLE_IS_VALID(FILEHANDLE) (FILEHANDLE == INVALID_HANDLE_VALUE)

typedef enum {
    IO_OK,
    IO_ERR_INVALID_ARGS,
    IO_ERR_FAILED_TO_OPEN,
    IO_ERR_FAILED_TO_READ,
    IO_ERR_FAILED_TO_OFFSET_HANDLE,
    IO_ERR_READED_LESS_THAN_REQUIRED,
} IOResult;

typedef u8 IOPermissions;

#define IO_READ MKFLAG(1)
#define IO_WRITE MKFLAG(2)
#define IO_READ_WRITE IO_READ | IO_WRITE

IOResult IOOpenFile(cstring8 filePath, FileHandle *handleOut, IOPermissions perms);
IOResult IOLoadBytesFromFile(FileHandle *handle, void *buffer, usize numberOfBytes);
IOResult IOLoadBytesFromFileEx(FileHandle *handle, void *buffer, usize numberOfBytes, usize offset);

#endif // GFS_IO_H_INCLUDED
