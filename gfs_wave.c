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
WaveAssetLoadFromFile(ScratchAllocator *scratchAllocator, CStr8 assetPath, WaveAsset *waveAssetOut) {
    if (scratchAllocator == NULL || assetPath == NULL || waveAssetOut == NULL || CStr8IsEmpty(assetPath)) {
        return WAVEASSET_LOAD_ERR_INVALID_ARGS;
    }

    if (!FSIsPathExists(assetPath)) {
        return WAVEASSET_LOAD_ERR_FILE_NOT_FOUND;
    }

    FileHandle assetFileHandle;
    IOResult openAssetFileResult = IOOpenFile(assetPath, &assetFileHandle, IO_READ);

    if (openAssetFileResult != IO_OK) {
        return WAVEASSET_LOAD_ERR_FAILED_TO_OPEN;
    }

    WaveFileHeader header;

    IOResult loadFromAssetFile;

    loadFromAssetFile = IOLoadBytesFromFile(&assetFileHandle, &header, sizeof(header));

    if (loadFromAssetFile != IO_OK) {
        return WAVEASSET_LOAD_ERR_FAILED_TO_READ;
    }

    // TODO(ilya.a): Implement magic check [2024/06/08]
    // if (!CStr8IsEqual(header.FileTypeBlocID, WAVEFILE_FILETYPE) ||
    //     !CStr8IsEqual(header.FileFormatID,   WAVEFILE_FORMATID) ||
    //     !CStr8IsEqual(header.FormatBlocID,   WAVEFILE_FORMATBLOCID) ||
    //     !CStr8IsEqual(header.DataBlocID,     WAVEFILE_DATABLOCID))
    // {
    //     return WAVEASSET_LOAD_ERR_INVALID_MAGIC;
    // }

    Void *data = ScratchAllocatorAlloc(scratchAllocator, header.DataSize);

    if (data == NULL) {
        return WAVEASSET_LOAD_ERR_FAILED_TO_ALLOC;
    }

    loadFromAssetFile = IOLoadBytesFromFileEx(&assetFileHandle, data, header.DataSize, sizeof(header));

    if (loadFromAssetFile != IO_OK) {
        return WAVEASSET_LOAD_ERR_FAILED_TO_READ;
    }

    waveAssetOut->Header = header;
    waveAssetOut->Data = data;

    return WAVEASSET_LOAD_OK;
}

WaveAssetLoadResult WaveAssetLoadFromMemory(ScratchAllocator *arena, const Void *buffer, WaveAsset *waveAssetOut);

void WaveAssetFree(WaveAsset *wa);
