/*
 * FILE      gfs_game.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs_game.h"

#include "gfs_game_state.h"
#include "gfs_platform.h"
#include "gfs_memory.h"
#include "gfs_assert.h"
#include "gfs_render.h"

void
GameMainloop(Renderer *renderer) {
    ScratchAllocator platformScratch = ScratchAllocatorMake(KILOBYTES(1));

    PlatformWindow *window = PlatformWindowOpen(&platformScratch, 900, 600, "Hello world!");

#if 0
    PlatformSoundDevice *soundDevice = PlatformSoundDeviceOpen(&platformScratch, window);
#endif

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

        BeginDrawing(renderer);

        ClearBackground(renderer);
        DrawGradient(renderer, xOffset, yOffset);

#if 0
        DWORD playCursor;
        DWORD writeCursor;

        if (SUCCEEDED(VCALL(g_Win32_AudioBuffer, GetCurrentPosition, &playCursor, &writeCursor))) {
            DWORD byteToLock =
                (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) % soundOutput.audioBufferSize;
            DWORD targetCursor = (playCursor + (soundOutput.latencySampleCount * soundOutput.bytesPerSample)) %
                                 soundOutput.audioBufferSize;
            DWORD bytesToWrite = 0;

            if (byteToLock > targetCursor) {
                bytesToWrite = soundOutput.audioBufferSize - byteToLock;
                bytesToWrite += targetCursor;
            } else {
                bytesToWrite = targetCursor - byteToLock;
            }

            Win32_FillSoundBuffer(&soundOutput, byteToLock, bytesToWrite);
        }
#endif

        EndDrawing(renderer);

        xOffset++;
        yOffset++;

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
    ScratchAllocatorFree(&platformScratch);
}
