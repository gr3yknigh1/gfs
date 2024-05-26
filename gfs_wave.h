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
    WaveFileAudioFormat AudioFormat;            // Audio format (1: PCM integer, 3: IIEE float)
    U16 NumberOfChannels;                       // Number of channels
    U32 FreqHZ;                                 // Sample rate (in hertz)
    U32 BytePerSec;                             // Number of bytes to read per second (Frequence * BytePerBloc)
    U16 BytePerBloc;                            // Number of bytes per block (NbrChannels * BitsPerSample / 8)
    U16 BitsPerSample;                          // Number of bits per sample

    /* Chunk containing the sampled data */
    U32 DataBlocID;                             // Identifier "data"  (0x64, 0x61, 0x74, 0x61)
    U32 DataSize;                               // SampledData size
} WaveFileHeader;
#pragma pack(pop)

typedef struct
{
    WaveFileHeader header;
} WaveAsset;

typedef enum
{
    WAVEASSET_LOAD_OK,              // It's okey.
    WAVEASSET_LOAD_ERR,             // General error.
    WAVEASSET_LOAD_INVALID_ARGS,    // You typed invalid arguments.
    WAVEASSET_LOAD_FILE_NOT_FOUND,  // Path which you typed is not existing. 
}  WaveAssetLoadResult;

WaveAssetLoadResult WaveAssetLoadFromFile(Arena *arena, CStr8 assetPath, WaveAsset *waveAssetOut);
WaveAssetLoadResult WaveAssetLoadFromMemory(Arena *arena, const Void *buffer, WaveAsset *waveAssetOut);

void WaveAssetFree(WaveAsset *wa);

#endif // GFS_WAVE_H_INCLUDED
