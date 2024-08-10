/*
 * FILE      gfs_sys.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_sys.h"

#include <Windows.h>

#include "gfs_types.h"

usize
Sys_GetPageSize() {
    usize pageSize;

    SYSTEM_INFO systemInfo = {0};
    GetSystemInfo(&systemInfo);

    pageSize = systemInfo.dwPageSize;

    return pageSize;
}
