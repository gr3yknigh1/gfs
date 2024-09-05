/*
 * FILE      Code\WaveAudio.cpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "WaveAudio.hpp"

#include "Assert.hpp"
#include "Platform.hpp"
#include "Types.hpp"
#include "String.hpp"

WaveAssetLoadResult
WaveAssetLoadFromFile(ScratchAllocator *scratchAllocator, cstring8 assetPath, WaveAsset *waveAssetOut) {
    ASSERT_NONNULL(scratchAllocator);
    ASSERT_NONNULL(assetPath);
    ASSERT(!CString8IsEmpty(assetPath));

    if (!PlatformIsPathExists(assetPath)) {
        return WAVEASSET_LOAD_ERR_FILE_NOT_FOUND;
    }

    ///< Openning handle of assert.
    PlatformFileOpenResult assetOpenResult = PlatformFileOpenEx(assetPath, scratchAllocator, PLATFORM_PERMISSION_READ);
    if (assetOpenResult.code != PLATFORM_FILE_OPEN_OK) {
        return WAVEASSET_LOAD_ERR_FAILED_TO_OPEN;
    }
    PlatformFileHandle *assetFileHandle = assetOpenResult.handle;

    ///< Loading assert's header.
    WaveFileHeader header;
    PlatformFileLoadResult assertHeaderLoadResult = PlatformFileLoadToBuffer(
        assetFileHandle, &header, sizeof(header), NULL); // TODO(ilya.a): Check how many bytes was loaded [2024/08/31]
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
    void *data = ScratchAllocatorAlloc(scratchAllocator, header.dataSize);
    ASSERT_NONNULL(data);
    PlatformFileLoadResult assertDataLoadResult =
        PlatformFileLoadToBufferEx(assetFileHandle, data, header.dataSize, NULL, sizeof(header));
    if (assertDataLoadResult != PLATFORM_FILE_LOAD_OK) {
        return WAVEASSET_LOAD_ERR_FAILED_TO_READ;
    }

    waveAssetOut->header = header;
    waveAssetOut->data = data;
    return WAVEASSET_LOAD_OK;
}

WaveAssetLoadResult WaveAssetLoadFromMemory(ScratchAllocator *arena, const void *buffer, WaveAsset *waveAssetOut);

void WaveAssetFree(WaveAsset *wa);
