/*
 * FILE      gfs_game.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs_game.h"

#include <Windows.h>

#include <math.h> // sinf
                  // TODO(ilya.a): Replace with custom code [2024/06/08]

#include "gfs_game_state.h"
#include "gfs_platform.h"
#include "gfs_memory.h"
#include "gfs_assert.h"
#include "gfs_render.h"
#include "gfs_wave.h"

#define PI32 3.14159265358979323846f

static void
GameFillSoundBuffer(PlatformSoundDevice *device, PlatformSoundOutput *output, u32 byteToLock, u32 bytesToWrite) {
    ASSERT_NONNULL(device);

    void *region0, *region1;
    u32 region0Size, region1Size;

    PlatformSoundDeviceLockBuffer(device, byteToLock, bytesToWrite, &region0, &region0Size, &region1, &region1Size);

    // NOTE(ilya.a): Logicly, regions might be NULLs. Maybe except first. Temporarly comment-out second region. [2024/08/31]
    ASSERT_NONNULL(region0);
    // ASSERT_NONNULL(region1);

    u32 region0SampleCount = region0Size / output->bytesPerSample;
    i16 *sampleOut = (i16 *)region0;
    for (u32 sampleIndex = 0; sampleIndex < region0SampleCount; ++sampleIndex) {
        f32 sinePosition = 2.0f * PI32 * (f32)output->runningSampleIndex / (f32)output->wavePeriod;
        f32 sineValue = sinf(sinePosition);
        i16 sampleValue = (i16)(sineValue * output->toneVolume);
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
        ++output->runningSampleIndex;
    }

    DWORD region2SampleCount = region1Size / output->bytesPerSample;
    sampleOut = (i16 *)region1;
    for (u32 sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex) {
        f32 sinePosition = 2.0f * PI32 * (f32)output->runningSampleIndex / (f32)output->wavePeriod;
        f32 sineValue = sinf(sinePosition);
        i16 sampleValue = (i16)(sineValue * output->toneVolume);
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
        ++output->runningSampleIndex;
    }

    PlatformSoundDeviceUnlockBuffer(device, region0, region0Size, region1, region1Size);
}

void
GameMainloop(Renderer *renderer) {
    ScratchAllocator platformScratch = ScratchAllocatorMake(KILOBYTES(1));
    ScratchAllocator assetScratch = ScratchAllocatorMake(MEGABYTES(10));

    PlatformWindow *window = PlatformWindowOpen(&platformScratch, 900, 600, "Hello world!");
    ASSERT_NONNULL(window);

    WaveAsset musicAsset;
    WaveAssetLoadResult musicLoadResult = WaveAssetLoadFromFile(&assetScratch, ".\\Assets\\test_music_01.wav", &musicAsset);
    ASSERT_ISOK(musicLoadResult);

    PlatformSoundOutput soundOutput = PlatformSoundOutputMake(48000);
    PlatformSoundDevice *soundDevice = PlatformSoundDeviceOpen(&platformScratch, window, soundOutput.samplesPerSecond, soundOutput.audioBufferSize);
    GameFillSoundBuffer(soundDevice, &soundOutput, 0, soundOutput.latencySampleCount * soundOutput.bytesPerSample  /* output.audioBufferSize */);
    PlatformSoundDevicePlay(soundDevice);

    u32 xOffset = 0;
    u32 yOffset = 0;

    renderer->clearColor = COLOR_WHITE;

    LARGE_INTEGER performanceCounterFrequency = {0};
    ASSERT_NONZERO(QueryPerformanceFrequency(&performanceCounterFrequency));

    LARGE_INTEGER lastCounter = {0};
    ASSERT_NONZERO(QueryPerformanceCounter(&lastCounter));

    u64 lastCycleCount = __rdtsc();

    while (!GameStateShouldStop()) {
        PlatformPoolEvents(window);

        ///< Rendering
        BeginDrawing(renderer);

        ClearBackground(renderer);
        DrawGradient(renderer, xOffset, yOffset);

        EndDrawing(renderer);

        ///< Playing sound
        u32 playCursor;
        u32 writeCursor;

        ASSERT_ISOK(PlatformSoundDeviceGetCurrentPosition(soundDevice, &playCursor, &writeCursor));
        u32 byteToLock =
            (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) % soundOutput.audioBufferSize;
        u32 targetCursor = (playCursor + (soundOutput.latencySampleCount * soundOutput.bytesPerSample)) %
                             soundOutput.audioBufferSize;
        u32 bytesToWrite = 0;

        if (byteToLock > targetCursor) {
            bytesToWrite = soundOutput.audioBufferSize - byteToLock;
            bytesToWrite += targetCursor;
        } else {
            bytesToWrite = targetCursor - byteToLock;
        }

        GameFillSoundBuffer(soundDevice, &soundOutput, byteToLock, bytesToWrite);


        xOffset++;
        yOffset++;

        ///< Perfomance
        {
            u64 endCycleCount = __rdtsc();

            LARGE_INTEGER endCounter;
            ASSERT_NONZERO(QueryPerformanceCounter(&endCounter));

            // TODO(ilya.a): Display counter [2024/11/08]

            u64 cyclesElapsed = endCycleCount - lastCycleCount;
            LONGLONG counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
            u64 msPerFrame = (1000 * counterElapsed) / performanceCounterFrequency.QuadPart;
            u64 framesPerSeconds = performanceCounterFrequency.QuadPart / counterElapsed;
            u64 megaCyclesPerFrame = cyclesElapsed / (1000 * 1000);

            char8 printBuffer[KILOBYTES(1)];
            wsprintf(printBuffer, "%ums/f | %uf/s | %umc/f\n", msPerFrame, framesPerSeconds, megaCyclesPerFrame);
            OutputDebugString(printBuffer);
            lastCounter = endCounter;
            lastCycleCount = endCycleCount;
        }
    }

    PlatformWindowClose(window);
    PlatformSoundDeviceClose(soundDevice);
    ScratchAllocatorFree(&platformScratch);
    ScratchAllocatorFree(&assetScratch);
}
