/*
 * FILE      gfs_game.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs_game.h"

#include <Windows.h>
#include <glad/glad.h>

#include <math.h> // sinf
                  // TODO(ilya.a): Replace with custom code [2024/06/08]

#include "gfs_game_state.h"
#include "gfs_platform.h"
#include "gfs_memory.h"
#include "gfs_assert.h"
#include "gfs_render.h"
#include "gfs_wave.h"
#include "gfs_render_opengl.h"
#include "gfs_bmp.h"

#define PI32 3.14159265358979323846f

static void
GameFillSoundBuffer(
    SoundDevice *device, SoundOutput *output, u32 byteToLock,
    u32 bytesToWrite) {
    ASSERT_NONNULL(device);
    ASSERT_NONNULL(output);

    void *region0, *region1;
    u32 region0Size, region1Size;

    SoundDeviceLockBuffer(
        device, byteToLock, bytesToWrite, &region0, &region0Size, &region1,
        &region1Size);

    u32 region0SampleCount = region0Size / output->bytesPerSample;
    i16 *sampleOut = (i16 *)region0;
    for (u32 sampleIndex = 0; sampleIndex < region0SampleCount; ++sampleIndex) {
        f32 sinePosition = 2.0f * PI32 * (f32)output->runningSampleIndex /
                           (f32)output->wavePeriod;
        f32 sineValue = sinf(sinePosition);
        i16 sampleValue = (i16)(sineValue * output->toneVolume);
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
        ++output->runningSampleIndex;
    }

    u32 region2SampleCount = region1Size / output->bytesPerSample;
    sampleOut = (i16 *)region1;
    for (u32 sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex) {
        f32 sinePosition = 2.0f * PI32 * (f32)output->runningSampleIndex /
                           (f32)output->wavePeriod;
        f32 sineValue = sinf(sinePosition);
        i16 sampleValue = (i16)(sineValue * output->toneVolume);
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
        ++output->runningSampleIndex;
    }

    SoundDeviceUnlockBuffer(device, region0, region0Size, region1, region1Size);
}

static void
GameFillSoundBufferWaveAsset(
    SoundDevice *device, SoundOutput *output, WaveAsset *waveAsset,
    u32 byteToLock, u32 bytesToWrite) {
    ASSERT_NONNULL(device);
    ASSERT_NONNULL(output);
    ASSERT_NONNULL(waveAsset);

    // usize currentByteIndex = output->runningSampleIndex *
    // output->bytesPerSample;

    void *region0, *region1;
    u32 region0Size, region1Size;
    byte *sampleOut;

    SoundDeviceLockBuffer(
        device, byteToLock, bytesToWrite, &region0, &region0Size, &region1,
        &region1Size);

    u32 region0SampleCount = region0Size / output->bytesPerSample;
    sampleOut = (byte *)region0;
    for (u32 sampleIndex = 0; sampleIndex < region0SampleCount; ++sampleIndex) {
        // if (currentByteIndex > waveAsset->header.dataSize / 2) {
        //     output->runningSampleIndex = 0;
        // }
        MemoryCopy(
            sampleOut,
            (byte *)waveAsset->data +
                output->runningSampleIndex * output->bytesPerSample,
            output->bytesPerSample);
        sampleOut += output->bytesPerSample;
        output->runningSampleIndex++;
    }

    u32 region1SampleCount = region1Size / output->bytesPerSample;
    sampleOut = (byte *)region1;
    for (u32 sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex) {
        // if (currentByteIndex > waveAsset->header.dataSize / 2) {
        //     output->runningSampleIndex = 0;
        // }
        MemoryCopy(
            sampleOut,
            (byte *)waveAsset->data +
                output->runningSampleIndex * output->bytesPerSample,
            output->bytesPerSample);
        sampleOut += output->bytesPerSample;
        output->runningSampleIndex++;
    }

    SoundDeviceUnlockBuffer(device, region0, region0Size, region1, region1Size);
}

void
GameMainloop(Renderer *renderer) {
    Scratch runtimeScratch = ScratchMake(MEGABYTES(20));

    Window *window = WindowOpen(&runtimeScratch, 900, 600, "GameFromScratch");
    ASSERT_NONNULL(window);

    SoundOutput soundOutput = SoundOutputMake(48000);
    SoundDevice *soundDevice = SoundDeviceOpen(
        &runtimeScratch, window, soundOutput.samplesPerSecond,
        soundOutput.audioBufferSize);

    GameFillSoundBuffer(
        soundDevice, &soundOutput, 0,
        soundOutput.latencySampleCount * soundOutput.bytesPerSample);

    SoundDevicePlay(soundDevice);

    renderer->clearColor = COLOR_WHITE;

    LARGE_INTEGER performanceCounterFrequency = {0};
    ASSERT_NONZERO(QueryPerformanceFrequency(&performanceCounterFrequency));

    LARGE_INTEGER lastCounter = {0};
    ASSERT_NONZERO(QueryPerformanceCounter(&lastCounter));

    // TODO(gr3yknigh1): Destroy shaders after they are linked [2024/09/15]
    GLShaderProgramLinkData programData = {0};
    programData.vertexShader = GLCompileShaderFromFile(
        &runtimeScratch, "assets\\basic.frag.glsl", GL_SHADER_TYPE_FRAG);
    programData.fragmentShader = GLCompileShaderFromFile(
        &runtimeScratch, "assets\\basic.vert.glsl", GL_SHADER_TYPE_VERT);
    GLShaderProgramID shaderProgram =
        GLLinkShaderProgram(&runtimeScratch, &programData);
    ASSERT_NONZERO(shaderProgram);

    GLuint vao;
    GL_CALL(glGenVertexArrays(1, &vao));
    GL_CALL(glBindVertexArray(vao));

    static const f32 vertexBufferData[] = {
        -1.0f, -1.0f, 0.0f, //
        1.0f,  -1.0f, 0.0f, //
        0.0f,  1.0f,  0.0f, //
    };

    GLuint vbo;
    GL_CALL(glGenBuffers(1, &vbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferData(
        vbo, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW));

    GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    u64 lastCycleCount = __rdtsc();

    while (!GameStateShouldStop()) {

        PoolEvents(window);

        ///< Rendering
        BeginDrawing(renderer);

        GL_CALL(glUseProgram(shaderProgram));
        GL_CALL(glEnableVertexAttribArray(0));
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
        GL_CALL(glVertexAttribPointer(
            0, // attribute 0. No particular reason for 0, but must match the
               // layout in the shader.
            3, // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0,        // stride
            (void *)0 // array buffer offset
            ));

        GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));
        // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        // Starting from vertex 0; 3 vertices total -> 1 triangle
        //GL_CALL(glDisableVertexAttribArray(0));

        ClearBackground(renderer);
        EndDrawing(renderer);

        ///< Playing sound
        u32 playCursor = 0;
        u32 writeCursor = 0;

        ASSERT_ISOK(SoundDeviceGetCurrentPosition(
            soundDevice, &playCursor, &writeCursor));
        u32 byteToLock =
            (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) %
            soundOutput.audioBufferSize;
        u32 targetCursor = (playCursor + (soundOutput.latencySampleCount *
                                          soundOutput.bytesPerSample)) %
                           soundOutput.audioBufferSize;
        u32 bytesToWrite = 0;

        if (byteToLock > targetCursor) {
            bytesToWrite = soundOutput.audioBufferSize - byteToLock;
            bytesToWrite += targetCursor;
        } else {
            bytesToWrite = targetCursor - byteToLock;
        }

        if (bytesToWrite > 0) {
            GameFillSoundBuffer(
                soundDevice, &soundOutput, byteToLock, bytesToWrite);
        }

        ///< Perfomance
        {
            u64 endCycleCount = __rdtsc();

            LARGE_INTEGER endCounter;
            ASSERT_NONZERO(QueryPerformanceCounter(&endCounter));

            // TODO(ilya.a): Display counter [2024/11/08]

            u64 cyclesElapsed = endCycleCount - lastCycleCount;
            LONGLONG counterElapsed =
                endCounter.QuadPart - lastCounter.QuadPart;
            u64 msPerFrame =
                (1000 * counterElapsed) / performanceCounterFrequency.QuadPart;
            u64 framesPerSeconds =
                performanceCounterFrequency.QuadPart / counterElapsed;
            u64 megaCyclesPerFrame = cyclesElapsed / (1000 * 1000);

            char8 printBuffer[KILOBYTES(1)];
            wsprintf(
                printBuffer, "%ums/f | %uf/s | %umc/f\n", msPerFrame,
                framesPerSeconds, megaCyclesPerFrame);
            OutputDebugString(printBuffer);
            lastCounter = endCounter;
            lastCycleCount = endCycleCount;
        }
    }

    WindowClose(window);
    SoundDeviceClose(soundDevice);
    ScratchDestroy(&runtimeScratch);
}
