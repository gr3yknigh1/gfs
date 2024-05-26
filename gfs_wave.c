/*
 * FILE      gfs_wave.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_wave.h"

#include "gfs_types.h"
#include "gfs_string.h"
#include "gfs_fs.h"

WaveAssetLoadResult
WaveAssetLoadFromFile(Arena *arena, CStr8 assetPath, WaveAsset *waveAssetOut)
{
    if (arena == NULL || assetPath == NULL || waveAssetOut == NULL || CStr8_IsEmpty(assetPath))
    {
        return WAVEASSET_LOAD_INVALID_ARGS;
    }

    if (!IsPathExists(assetPath))
    {
        return WAVEASSET_LOAD_FILE_NOT_FOUND;
    }

    

    return WAVEASSET_LOAD_OK;
}

WaveAssetLoadResult WaveAssetLoadFromMemory(Arena *arena, const Void *buffer, WaveAsset *waveAssetOut);

void WaveAssetFree(WaveAsset *wa);
