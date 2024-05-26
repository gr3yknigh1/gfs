/*
 * FILE      gfs_wave.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_WAVE_H_INCLUDED
#define GFS_WAVE_H_INCLUDED

#include "gfs_types.h"
#include "gfs_memory.h"


typedef struct
{
    // data
} WaveAsset;


WaveAsset WaveAssetLoadFromFile(Arena *arena, const Char *assetPath);
WaveAsset WaveAssetLoadFromMemory(Arena *arena, const Void *buffer);
void      WaveAssetFree(WaveAsset *wa);


#endif // GFS_WAVE_H_INCLUDED
