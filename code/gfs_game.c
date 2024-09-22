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

#include <cglm/cglm.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>

#include "gfs_game_state.h"
#include "gfs_platform.h"
#include "gfs_memory.h"
#include "gfs_assert.h"
#include "gfs_render.h"
#include "gfs_types.h"
#include "gfs_wave.h"
#include "gfs_render_opengl.h"
#include "gfs_bmp.h"

#define PI32 3.14159265358979323846f

static void GameFillSoundBuffer(
    SoundDevice *device, SoundOutput *output, u32 byteToLock, u32 bytesToWrite);

static void GameFillSoundBufferWaveAsset(
    SoundDevice *device, SoundOutput *output, WaveAsset *waveAsset,
    u32 byteToLock, u32 bytesToWrite);

void
GameMainloop(Renderer *renderer) {
    UNUSED(renderer);

    Scratch runtimeScratch = ScratchMake(MEGABYTES(20));

    Window *window = WindowOpen(&runtimeScratch, 900, 600, "GameFromScratch");
    ASSERT_NONNULL(window);

    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    SoundOutput soundOutput = SoundOutputMake(48000);
    SoundDevice *soundDevice = SoundDeviceOpen(
        &runtimeScratch, window, soundOutput.samplesPerSecond,
        soundOutput.audioBufferSize);

    GameFillSoundBuffer(
        soundDevice, &soundOutput, 0,
        soundOutput.latencySampleCount * soundOutput.bytesPerSample);

    SoundDevicePlay(soundDevice);

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
    GLShaderProgramID shader =
        GLLinkShaderProgram(&runtimeScratch, &programData);
    ASSERT_NONZERO(shader);

#if 0
    static const f32 vertices[] = {
        // positions        // colors         // texture coords
        0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
        0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
    };

    static const u32 indices[] = {0, 1, 2, 0, 2, 3};
#else
    static const f32 vertices[] = {
        // l_Position        // l_Color        // l_TexCoord
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //
        0.5f,  -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, //
        0.5f,  0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, //
        0.5f,  0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, //
        -0.5f, 0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //
        0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, //
        0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, //
        0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, //
        -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //
        -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, //
        -0.5f, 0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, //
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //
        -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, //
        0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, //
        0.5f,  0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, //
        0.5f,  -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
        0.5f,  -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
        0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //
        0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, //
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
        0.5f,  -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, //
        0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, //
        0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, //
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
        -0.5f, 0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
        0.5f,  0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, //
        0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, //
        0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, //
        -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //
        -0.5f, 0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f  //
    };

    static const u32 indices[] = {0, 1, 2, 0, 2, 3};

#endif

    BMPicture picture = {0};
    ASSERT_ISOK(
        BMPictureLoadFromFile(&picture, &runtimeScratch, "assets\\dirt.bmp"));

    GLTexture texture = GLTextureMakeFromBMPicture(&picture);

    GLVertexArray va = GLVertexArrayMake();
    GLVertexBuffer vb = GLVertexBufferMake(vertices, sizeof(vertices));

    GLIndexBuffer ib = GLIndexBufferMake(indices, sizeof(indices));
    UNUSED(ib);

    GLVertexBufferLayout vbLayout = GLVertexBufferLayoutMake(&runtimeScratch);
    GLVertexBufferLayoutPushAttributeF32(&vbLayout, 3);
    GLVertexBufferLayoutPushAttributeF32(&vbLayout, 3);
    GLVertexBufferLayoutPushAttributeF32(&vbLayout, 2);

    GLVertexArrayAddBuffer(va, &vb, &vbLayout);

    GLShaderSetUniformF32(shader, "u_VertexModifier", 1.0f);
    GLShaderSetUniformV3F32(shader, "u_VertexOffset", 0.3f, 0.3f, 0.3f);
    GLShaderSetUniformI32(shader, "u_Texture", 0);

    vec3 cameraPosition = {0, 0, 3.0f};
    vec3 cameraFront = {0, 0, -1.0f};
    vec3 cameraUp = {0, 1.0, 0};

    // vec3 cameraTarget = {0, 0, 0};
    // vec3 cameraDirection = {0};
    // glm_vec3_sub(cameraPosition, cameraTarget, cameraDirection);
    // glm_vec3_normalize(cameraDirection);

    // vec3 upDirection = {0, 1, 0};
    // vec3 cameraRight = {0};
    // glm_vec3_cross(upDirection, cameraDirection, cameraRight);

    u64 lastCycleCount = __rdtsc();

    f32 dt = 0.0f;
    f32 cameraSpeed = 0.05f;

    while (!GameStateShouldStop()) {
        PoolEvents(window);

        {
            if (IsKeyDown(KEY_W)) {
                vec3 v = {0};
                glm_vec3_fill(v, cameraSpeed);

                glm_vec3_mul(v, cameraFront, v);
                glm_vec3_add(cameraPosition, v, cameraPosition);
            }

            if (IsKeyDown(KEY_S)) {
                vec3 v = {0};
                glm_vec3_fill(v, cameraSpeed);

                glm_vec3_mul(v, cameraFront, v);
                glm_vec3_sub(cameraPosition, v, cameraPosition);
            }

            if (IsKeyDown(KEY_A)) {
                vec3 v = {0};
                glm_vec3_fill(v, cameraSpeed);

                vec3 cameraDirection = {0};
                glm_vec3_cross(cameraFront, cameraUp, cameraDirection);
                glm_vec3_normalize(cameraDirection);
                glm_vec3_mul(cameraDirection, v, cameraDirection);

                glm_vec3_sub(cameraPosition, cameraDirection, cameraPosition);
            }

            if (IsKeyDown(KEY_D)) {
                vec3 v = {0};
                glm_vec3_fill(v, cameraSpeed);

                vec3 cameraDirection = {0};
                glm_vec3_cross(cameraFront, cameraUp, cameraDirection);
                glm_vec3_normalize(cameraDirection);
                glm_vec3_mul(cameraDirection, v, cameraDirection);

                glm_vec3_add(cameraPosition, cameraDirection, cameraPosition);
            }
        }

        GLClear(0, 0, 0, 1); // TODO: Map from 0..255 to 0..1

        mat4 model = {0};
        {
            vec3 xAxisVec = {1.0f, 0, 0};
            glm_mat4_make(GLM_MAT4_IDENTITY, model);
            glm_rotate(model, glm_rad(0.01f * dt), xAxisVec);
        }

        mat4 view = {0};
        {
            vec3 cameraCenter = {0};
            glm_vec3_add(cameraPosition, cameraFront, cameraCenter);
            glm_lookat(cameraPosition, cameraCenter, cameraUp, view);
        }

        mat4 projection = {0};
        {
            f32 fov = 45.0f;

            RectangleI32 windowRect = WindowGetRectangle(window);

            glm_perspective(
                fov, (f32)windowRect.width / (f32)windowRect.height, 0.1f,
                //   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                // NOTE(gr3yknigh1): Viewport width and height (not the window).
                // [2024/09/22]
                100.0f, projection);
        }

        GLShaderSetUniformM4F32(shader, "u_Model", (f32 *)model);
        GLShaderSetUniformM4F32(shader, "u_View", (f32 *)view);
        GLShaderSetUniformM4F32(shader, "u_Projection", (f32 *)projection);

        GLDrawTriangles(&vb, &vbLayout, va, shader, texture);

        WindowUpdate(window);

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

            // TODO(ilya.a): Display counter [2025/11/08]

            u64 cyclesElapsed = endCycleCount - lastCycleCount;
            LONGLONG counterElapsed =
                endCounter.QuadPart - lastCounter.QuadPart;

            dt = (f32)(1000LL * endCounter.QuadPart) /
                 performanceCounterFrequency.QuadPart;

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
