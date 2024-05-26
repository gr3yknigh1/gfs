/*
 * FILE      gfs_wave.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_WAVE_H_INCLUDED
#define GFS_WAVE_H_INCLUDED

#include "gfs_types.h"
#include "gfs_memory.h"

typedef enum
{
    WAVEFILE_AUDIOFORMAT_PCM      = 1,
    WAVEFILE_AUDIOFORMAT_IIEE_FLT = 3,
} WaveFileAudioFormat;

/* NOTE(ilya.a): This piece of code just for the sake of the shame 
 * I have with MSVC and C language. Which doesn't support fixed-size enums. [2024/05/26]
 */
typedef U16 WaveFileAudioFormatTag;

#pragma pack(push, 1)
/*
 * Wave file header. 
 * Should be 44 bytes.
 * 
 * Using as a reference this page: https://en.wikipedia.org/wiki/WAV
 */
typedef struct
{
    /* Master RIFF chunk */
    Char8 FileTypeBlocID[4];                    // Identifier [RIFF]  (0x52, 0x49, 0x46, 0x46)
    U32   FileSize;                             // Overall file size minus 8 bytes
    Char8 FileFormatID[4];                      // Format = [WAVE]  (0x57, 0x41, 0x56, 0x45)

    /* Chunk describing the data format */
    Char8 FormatBlocID[4];                      // Identifier [fmt ]  (0x66, 0x6D, 0x74, 0x20)
    U32 BlockSize;                              // Chunk size minus 8 bytes, which is 16 bytes here  (0x10)
    WaveFileAudioFormatTag AudioFormat;         // Audio format (1: PCM integer, 3: IIEE float)
    U16 NumberOfChannels;                       // Number of channels
    U32 FreqHZ;                                 // Sample rate (in hertz)
    U32 BytePerSec;                             // Number of bytes to read per second (Frequence * BytePerBloc)
    U16 BytePerBloc;                            // Number of bytes per block (NbrChannels * BitsPerSample / 8)
    U16 BitsPerSample;                          // Number of bits per sample

    /* Chunk containing the sampled data */
    Char8 DataBlocID[4];                        // Identifier "data"  (0x64, 0x61, 0x74, 0x61)
    U32 DataSize;                               // SampledData size
} WaveFileHeader;
#pragma pack(pop)

typedef struct
{
    WaveFileHeader header;
    Void *data;
} WaveAsset;

typedef enum
{
    WAVEASSET_LOAD_OK,                  // It's okey.
    WAVEASSET_LOAD_ERR,                 // General error.
    WAVEASSET_LOAD_ERR_INVALID_ARGS,    // You typed invalid arguments.
    WAVEASSET_LOAD_ERR_FILE_NOT_FOUND,  // Path which you typed is not existing. 
    WAVEASSET_LOAD_ERR_FAILED_TO_OPEN,  // IO error. Failed to open the asset file.
    WAVEASSET_LOAD_ERR_FAILED_TO_READ,  // IO error. Opened the file, but failed to read it.
    WAVEASSET_ERR_LOAD_FAILED_TO_ALLOC, // Out of memory with Arena.
}  WaveAssetLoadResult;

WaveAssetLoadResult WaveAssetLoadFromFile(Arena *arena, CStr8 assetPath, WaveAsset *waveAssetOut);
WaveAssetLoadResult WaveAssetLoadFromMemory(Arena *arena, const Void *buffer, WaveAsset *waveAssetOut);

void WaveAssetFree(WaveAsset *wa);

#endif // GFS_WAVE_H_INCLUDED
