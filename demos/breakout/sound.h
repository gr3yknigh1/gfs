#if !defined(BREAKOUT_SOUND_H_INCLUDED)
/*
 * FILE      demos\breakout\sound.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define BREAKOUT_SOUND_H_INCLUDED

#include <gfs/platform.h>
#include <gfs/types.h>
#include <gfs/wave.h>

void GameFillSoundBuffer(
    SoundDevice *device, SoundOutput *output, u32 byteToLock, u32 bytesToWrite);
void GameFillSoundBufferWaveAsset(
    SoundDevice *device, SoundOutput *output, WaveAsset *waveAsset,
    u32 byteToLock, u32 bytesToWrite);

#endif // BREAKOUT_SOUND_H_INCLUDED
