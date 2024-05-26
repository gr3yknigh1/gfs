/*
 * FILE      gfs_wave.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_wave.h"

#include "gfs_types.h"
#include "gfs_string.h"
#include "gfs_fs.h"
#include "gfs_io.h"


WaveAssetLoadResult
WaveAssetLoadFromFile(Arena *arena, CStr8 assetPath, WaveAsset *waveAssetOut)
{
    if (arena == NULL || assetPath == NULL || waveAssetOut == NULL || CStr8_IsEmpty(assetPath))
    {
        return WAVEASSET_LOAD_ERR_INVALID_ARGS;
    }

    if (!FSIsPathExists(assetPath))
    {
        return WAVEASSET_LOAD_ERR_FILE_NOT_FOUND;
    }

    FileHandle assetFileHandle;
    IOResult openAssetFileResult = IOOpenFile(assetPath, &assetFileHandle, IO_READ);

    if (openAssetFileResult != IO_OK)
    {
        return WAVEASSET_LOAD_ERR_FAILED_TO_OPEN;
    }

    WaveFileHeader header;

    IOResult loadFromAssetFile;

    loadFromAssetFile = IOLoadBytesFromFile(&assetFileHandle, &header, sizeof(header));
    
    if (loadFromAssetFile != IO_OK)
    {
        return WAVEASSET_LOAD_ERR_FAILED_TO_READ;
    }

    Void *data = ArenaAlloc(arena, header.DataSize);

    if (data == NULL)
    {
        return WAVEASSET_ERR_LOAD_FAILED_TO_ALLOC;
    }

    loadFromAssetFile = IOLoadBytesFromFileEx(&assetFileHandle, data, header.DataSize, sizeof(header));
    
    if (loadFromAssetFile != IO_OK)
    {
        return WAVEASSET_LOAD_ERR_FAILED_TO_READ;
    }

    waveAssetOut->header = header;
    waveAssetOut->data = data;

    return WAVEASSET_LOAD_OK;
}

WaveAssetLoadResult WaveAssetLoadFromMemory(Arena *arena, const Void *buffer, WaveAsset *waveAssetOut);

void WaveAssetFree(WaveAsset *wa);
