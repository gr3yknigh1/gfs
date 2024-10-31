#if !defined(GFS_WAVE_H_INCLUDED)
/*
 * FILE      gfs_wave.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_WAVE_H_INCLUDED

#include "gfs/assert.h"
#include "gfs/types.h"
#include "gfs/memory.h"

#define WAVEFILE_FILETYPE "RIFF"
#define WAVEFILE_FORMATID "WAVE"
#define WAVEFILE_FORMATBLOCID "fmt "
#define WAVEFILE_DATABLOCID "data"

typedef enum {
    WAVEFILE_AUDIOFORMAT_PCM = 1,
    WAVEFILE_AUDIOFORMAT_IIEE_FLT = 3,
} WaveFileAudioFormat;

/* NOTE(ilya.a): This piece of code just for the sake of the shame
 * I have with MSVC and C language. Which doesn't support fixed-size enums.
 * [2024/05/26]
 */
typedef u16 WaveFileAudioFormatTag;

#define WAVEFILE_HEADER_SIZE 44

#pragma pack(push, 1)
/*
 * Wave file header.
 * Should be 44 bytes.
 *
 * Using as a reference this page: https://en.wikipedia.org/wiki/WAV
 */
typedef struct {
    /* Master RIFF chunk */
    char8 fileTypeBlocID[4]; // Identifier [RIFF]  (0x52, 0x49, 0x46, 0x46)
    u32 fileSize;            // Overall file size minus 8 bytes
    char8 fileFormatID[4];   // Format = [WAVE]  (0x57, 0x41, 0x56, 0x45)

    /* Chunk describing the data format */
    char8 formatBlocID[4]; // Identifier [fmt ]  (0x66, 0x6D, 0x74, 0x20)
    u32 blockSize; // Chunk size minus 8 bytes, which is 16 bytes here  (0x10)
    WaveFileAudioFormatTag
        audioFormat;      // Audio format (1: PCM integer, 3: IIEE float)
    u16 numberOfChannels; // Number of channels
    u32 freqHZ;           // Sample rate (in hertz)
    u32 bytePerSec;       // Number of bytes to read per second (Frequence *
                          // BytePerBloc)
    u16 bytePerBloc; // Number of bytes per block (NbrChannels * BitsPerSample /
                     // 8)
    u16 bitsPerSample; // Number of bits per sample

    /* Chunk containing the sampled data */
    char8 dataBlocID[4]; // Identifier "data"  (0x64, 0x61, 0x74, 0x61)
    u32 dataSize;        // SampledData size
} WaveFileHeader;
#pragma pack(pop)

EXPECT_TYPE_SIZE(WaveFileHeader, WAVEFILE_HEADER_SIZE);

typedef struct {
    WaveFileHeader header;
    void *data;
} WaveAsset;

typedef enum {
    WAVEASSET_LOAD_OK,                 // It's okey.
    WAVEASSET_LOAD_ERR,                // General error.
    WAVEASSET_LOAD_ERR_FILE_NOT_FOUND, // Path which you typed is not existing.
    WAVEASSET_LOAD_ERR_FAILED_TO_OPEN, // IO error. Failed to open the asset
                                       // file.
    WAVEASSET_LOAD_ERR_FAILED_TO_READ, // IO error. Opened the file, but failed
                                       // to read it.
    WAVEASSET_LOAD_ERR_INVALID_MAGIC,  // Asset failed signature checks.
} WaveAssetLoadResult;

WaveAssetLoadResult WaveAssetLoadFromFile(
    Scratch *arena, cstring8 assetPath, WaveAsset *waveAssetOut);
WaveAssetLoadResult WaveAssetLoadFromMemory(
    Scratch *arena, const void *buffer, WaveAsset *waveAssetOut);

void WaveAssetFree(WaveAsset *wa);

#endif // GFS_WAVE_H_INCLUDED
