/*
 * FILE      gfs_wave.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_wave.h"

#include "gfs_assert.h"
#include "gfs_platform.h"
#include "gfs_types.h"
#include "gfs_string.h"

WaveAssetLoadResult
WaveAssetLoadFromFile(
    Scratch *scratchAllocator, cstring8 assetPath, WaveAsset *waveAssetOut) {
    ASSERT_NONNULL(scratchAllocator);
    ASSERT_NONNULL(assetPath);
    ASSERT(!CString8IsEmpty(assetPath));

    if (!IsPathExists(assetPath)) {
        return WAVEASSET_LOAD_ERR_FILE_NOT_FOUND;
    }

    ///< Openning handle of assert.
    FileOpenResult assetOpenResult =
        FileOpenEx(assetPath, scratchAllocator, PLATFORM_PERMISSION_READ);
    if (assetOpenResult.code != PLATFORM_FILE_OPEN_OK) {
        return WAVEASSET_LOAD_ERR_FAILED_TO_OPEN;
    }
    FileHandle *assetFileHandle = assetOpenResult.handle;

    ///< Loading assert's header.
    WaveFileHeader header;
    FileLoadResultCode assertHeaderLoadResult = FileLoadToBuffer(
        assetFileHandle, &header, sizeof(header),
        NULL); // TODO(ilya.a): Check how many bytes was loaded [2024/08/31]
    if (assertHeaderLoadResult != PLATFORM_FILE_LOAD_OK) {
        return WAVEASSET_LOAD_ERR_FAILED_TO_READ;
    }

    ///< Checking magic.
    // TODO(ilya.a): Implement magic check [2024/06/08]
    // if (!CStr8IsEqual(header.FileTypeBlocID, WAVEFILE_FILETYPE) ||
    //     !CStr8IsEqual(header.FileFormatID,   WAVEFILE_FORMATID) ||
    //     !CStr8IsEqual(header.FormatBlocID,   WAVEFILE_FORMATBLOCID) ||
    //     !CStr8IsEqual(header.DataBlocID,     WAVEFILE_DATABLOCID))
    // {
    //     return WAVEASSET_LOAD_ERR_INVALID_MAGIC;
    // }

    ///< Loading body of the asset.
    void *data = ScratchAlloc(scratchAllocator, header.dataSize);
    ASSERT_NONNULL(data);
    FileLoadResultCode assertDataLoadResult = FileLoadToBufferEx(
        assetFileHandle, data, header.dataSize, NULL, sizeof(header));
    if (assertDataLoadResult != PLATFORM_FILE_LOAD_OK) {
        return WAVEASSET_LOAD_ERR_FAILED_TO_READ;
    }

    waveAssetOut->header = header;
    waveAssetOut->data = data;
    return WAVEASSET_LOAD_OK;
}

WaveAssetLoadResult WaveAssetLoadFromMemory(
    Scratch *arena, const void *buffer, WaveAsset *waveAssetOut);

void WaveAssetFree(WaveAsset *wa);
