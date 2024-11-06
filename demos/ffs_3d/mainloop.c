/*
 * Fully From Scratch 3D.
 *
 * Trying to get working windowing, music and other 3D stuff on Windows API.
 *
 * FILE      demos\ffs_3d\mainloop.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include <Windows.h>
#include <glad/glad.h>

#include <stdio.h>

#include <cglm/cglm.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>

#include <gfs/entry.h>
#include <gfs/game_state.h>
#include <gfs/platform.h>
#include <gfs/memory.h>
#include <gfs/assert.h>
#include <gfs/render.h>
#include <gfs/types.h>
#include <gfs/wave.h>
#include <gfs/render_opengl.h>
#include <gfs/bmp.h>

#define PI32 3.14159265358979323846f

typedef struct {
    vec3 position;
    vec3 front;
    vec3 up;

    f32 yaw;
    f32 pitch;

    f32 speed;
    f32 sensitivity;
    f32 fov;

    Window *window;
} Camera;

static void GameFillSoundBuffer(
    SoundDevice *device, SoundOutput *output, u32 byteToLock, u32 bytesToWrite);
static void GameFillSoundBufferWaveAsset(
    SoundDevice *device, SoundOutput *output, WaveAsset *waveAsset,
    u32 byteToLock, u32 bytesToWrite);

static Camera CameraMake(Window *window);
static void CameraRotate(Camera *camera, f32 xOffset, f32 yOffset);
static void CameraHandleInput(Camera *camera);
static void CameraGetViewMatix(Camera *camera, mat4 *view);
static void CameraGetProjectionMatix(Camera *camera, mat4 *projection);

static f32 ClampF32(f32 value, f32 min, f32 max);

void
Entry(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    Scratch runtimeScratch = ScratchMake(MEGABYTES(20));

    Window *window = WindowOpen(&runtimeScratch, 900, 600, "GameFromScratch");
    ASSERT_NONNULL(window);

    RectangleI32 windowRect = WindowGetRectangle(window);
    // SetMouseVisibility(MOUSEVISIBILITYSTATE_HIDDEN);
    SetMousePosition(window, windowRect.width / 2, windowRect.height / 2);

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
    GLShaderProgramLinkData shaderLinkData = {0};
    shaderLinkData.vertexShader = GLCompileShaderFromFile(
        &runtimeScratch, "P:\\gfs\\assets\\basic.frag.glsl",
        GL_SHADER_TYPE_FRAG);
    shaderLinkData.fragmentShader = GLCompileShaderFromFile(
        &runtimeScratch, "P:\\gfs\\assets\\basic.vert.glsl",
        GL_SHADER_TYPE_VERT);
    GLShaderProgramID shader =
        GLLinkShaderProgram(&runtimeScratch, &shaderLinkData);
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
    ASSERT_ISOK(BMPictureLoadFromFile(
        &picture, &runtimeScratch, "P:\\gfs\\assets\\kitty.bmp"));

    GLTexture texture = GLTextureMakeFromBMPicture(&picture, COLOR_LAYOUT_BGR);

    GLVertexArray va = GLVertexArrayMake();
    GLVertexBuffer vb = GLVertexBufferMake(vertices, sizeof(vertices));

    GLIndexBuffer ib = GLIndexBufferMake(indices, sizeof(indices));
    UNUSED(ib);

    GLVertexBufferLayout vbLayout = GLVertexBufferLayoutMake(&runtimeScratch);
    GLVertexBufferLayoutPushAttributeF32(&vbLayout, 3);
    GLVertexBufferLayoutPushAttributeF32(&vbLayout, 3);
    GLVertexBufferLayoutPushAttributeF32(&vbLayout, 2);

    GLVertexArrayAddBuffer(va, &vb, &vbLayout);

    GLUniformLocation uniformVertexModifierLocation =
        GLShaderFindUniformLocation(shader, "u_VertexModifier");
    GLUniformLocation uniformVertexOffsetLocation =
        GLShaderFindUniformLocation(shader, "u_VertexOffset");
    GLUniformLocation uniformTextureLocation =
        GLShaderFindUniformLocation(shader, "u_Texture");

    GLUniformLocation uniformModelLocation =
        GLShaderFindUniformLocation(shader, "u_Model");
    GLUniformLocation uniformViewLocation =
        GLShaderFindUniformLocation(shader, "u_View");
    GLUniformLocation uniformProjectionLocation =
        GLShaderFindUniformLocation(shader, "u_Projection");

    GLShaderSetUniformF32(shader, uniformVertexModifierLocation, 1.0f);
    GLShaderSetUniformV3F32(
        shader, uniformVertexOffsetLocation, 0.3f, 0.3f, 0.3f);
    GLShaderSetUniformI32(shader, uniformTextureLocation, 0);

    Camera camera = CameraMake(window);

    f32 dt = 0.0f;
    u64 lastCycleCount = __rdtsc();

    bool isFirstMainloopIteration = true;
    i32 lastMouseXPosition = 0;
    i32 lastMouseYPosition = 0;

    GL_CALL(glUseProgram(shader));
    GL_CALL(
        glActiveTexture(GL_TEXTURE0)); // TODO(gr3yknigh1): Investigate in multi
                                       // texture support. [2024/09/22]
    GL_CALL(glBindTexture(GL_TEXTURE_2D, texture));

    while (!GameStateShouldStop()) {
        PoolEvents(window);

        windowRect = WindowGetRectangle(window);
        Vector2I32 mousePosition = GetMousePosition(window);

        if (isFirstMainloopIteration) {
            lastMouseXPosition = mousePosition.x;
            lastMouseYPosition = mousePosition.y;
        }

        f32 mouseXOffset = (f32)mousePosition.x - lastMouseXPosition;
        f32 mouseYOffset = (f32)lastMouseYPosition - mousePosition.y;

        lastMouseXPosition = mousePosition.x;
        lastMouseYPosition = mousePosition.y;

        CameraRotate(&camera, mouseXOffset, mouseYOffset);
        CameraHandleInput(&camera);

        GLClear(0, 0, 0, 1); // TODO: Map from 0..255 to 0..1

        mat4 model = {0};
        glm_mat4_copy(GLM_MAT4_IDENTITY, model);
#if 0
        vec3 xAxisVec = {1.0f, 0, 0};
        glm_rotate(model, 0 /* glm_rad(0.01f * dt) */, xAxisVec);
#endif

        mat4 view = {0};
        CameraGetViewMatix(&camera, &view);

        mat4 projection = {0};
        CameraGetProjectionMatix(&camera, &projection);

        GLShaderSetUniformM4F32(shader, uniformModelLocation, (f32 *)model);
        GLShaderSetUniformM4F32(shader, uniformViewLocation, (f32 *)view);
        GLShaderSetUniformM4F32(
            shader, uniformProjectionLocation, (f32 *)projection);

        GLDrawTriangles(&vb, &vbLayout, va);

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
            sprintf(
                printBuffer,
                "%llums/f | %lluf/s | %llumc/f || mouse x=%d y=%d || camera "
                "yaw=%.3f pitch=%.3f || mouseOffset=[%.3f "
                "%.3f] || lastPos=[%d %d]\n",
                msPerFrame, framesPerSeconds, megaCyclesPerFrame,
                mousePosition.x, mousePosition.y, camera.yaw, camera.pitch,
                mouseXOffset, mouseYOffset, lastMouseXPosition,
                lastMouseYPosition);
            OutputDebugString(printBuffer);
            lastCounter = endCounter;
            lastCycleCount = endCycleCount;
        }

        if (isFirstMainloopIteration) {
            isFirstMainloopIteration = false;
        }
    }

    WindowClose(window);
    SoundDeviceClose(soundDevice);
    ScratchDestroy(&runtimeScratch);
}

static void
GameFillSoundBuffer(
    SoundDevice *device, SoundOutput *output, u32 byteToLock, u32 bytesToWrite)
{
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
        f32 sineValue = sinf(100 * sinePosition);
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
        f32 sineValue = sinf(100 * sinePosition);
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
    u32 byteToLock, u32 bytesToWrite)
{
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

static Camera
CameraMake(Window *window)
{
    UNUSED(window);

    Camera camera = {
        .position = {0, 0, 3.0f},
        .front = {0, 0, -1.0f},
        .up = {0, 1.0f, 0},

        .yaw = -90.0f,
        .pitch = 0.0f,

        .speed = 0.05f,
        .sensitivity = 0.1f,
        .fov = 45.0f,

        .window = window,
    };
    return camera;
}

static void
CameraRotate(Camera *camera, f32 xOffset, f32 yOffset)
{
    xOffset *= camera->sensitivity;
    yOffset *= camera->sensitivity;

    camera->yaw += xOffset;
    camera->pitch += yOffset;

    camera->pitch = ClampF32(camera->pitch, -89.0f, 89.0f);

    vec3 direction = {0};

    f32 yawRad = glm_rad(camera->yaw);
    f32 pitchRad = glm_rad(camera->pitch);
    direction[0] = cosf(yawRad) * cosf(pitchRad);
    direction[1] = sinf(pitchRad);
    direction[2] = sinf(yawRad) * cosf(pitchRad);

    glm_normalize_to(direction, camera->front);
}

static void
CameraHandleInput(Camera *camera)
{
    if (IsKeyDown(KEY_W)) {
        vec3 v = {0};
        glm_vec3_fill(v, camera->speed);

        glm_vec3_mul(v, camera->front, v);
        glm_vec3_add(camera->position, v, camera->position);
    }

    if (IsKeyDown(KEY_S)) {
        vec3 v = {0};
        glm_vec3_fill(v, camera->speed);

        glm_vec3_mul(v, camera->front, v);
        glm_vec3_sub(camera->position, v, camera->position);
    }

    if (IsKeyDown(KEY_A)) {
        vec3 v = {0};
        glm_vec3_fill(v, camera->speed);

        vec3 cameraDirection = {0};
        glm_vec3_cross(camera->front, camera->up, cameraDirection);
        glm_vec3_normalize(cameraDirection);
        glm_vec3_mul(cameraDirection, v, cameraDirection);

        glm_vec3_sub(camera->position, cameraDirection, camera->position);
    }

    if (IsKeyDown(KEY_D)) {
        vec3 v = {0};
        glm_vec3_fill(v, camera->speed);

        vec3 cameraDirection = {0};
        glm_vec3_cross(camera->front, camera->up, cameraDirection);
        glm_vec3_normalize(cameraDirection);
        glm_vec3_mul(cameraDirection, v, cameraDirection);

        glm_vec3_add(camera->position, cameraDirection, camera->position);
    }
}

static void
CameraGetViewMatix(Camera *camera, mat4 *view)
{
    vec3 center = {0};
    glm_vec3_add(camera->position, camera->front, center);

    glm_lookat(camera->position, center, camera->up, *view);
}

static void
CameraGetProjectionMatix(Camera *camera, mat4 *projection)
{
    RectangleI32 windowRect = WindowGetRectangle(camera->window);
    glm_perspective(
        glm_rad(camera->fov), (f32)windowRect.width / (f32)windowRect.height,
        0.1f,
        //                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        // NOTE(gr3yknigh1): Viewport width and height (not the window).
        // [2024/09/22]
        100.0f, *projection);
}

static f32
ClampF32(f32 value, f32 min, f32 max)
{
    if (value > max) {
        return max;
    }

    if (value < min) {
        return min;
    }

    return value;
}
