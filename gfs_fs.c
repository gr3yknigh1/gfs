/*
 * FILE      gfs_fs.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_fs.h"

#include <shlwapi.h>

#include "gfs_types.h"

Bool 
FSIsPathExists(CStr8 path)
{
    return PathFileExistsA(path);
}
