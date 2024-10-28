/*
 * Breakout Game.
 *
 * FILE      demos\breakout\breakout_entry.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include <stdio.h>
#include <math.h> // sinf cosf
                  // TODO(ilya.a): Replace with custom code [2024/06/08]

#include <Windows.h>
#undef far
#undef near

#include <glad/glad.h>

#include <gfs/entry.h>
#include <gfs/game_state.h>
#include <gfs/platform.h>
#include <gfs/memory.h>
#include <gfs/assert.h>
#include <gfs/render.h>
#include <gfs/types.h>
#include <gfs/render_opengl.h>
#include <gfs/bmp.h>
#include <gfs/string.h>

#include "breakout_render.h"
#include "breakout_sound.h"

void
Entry(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    Scratch runtimeScratch = ScratchMake(MEGABYTES(20));

    Window *window = WindowOpen(&runtimeScratch, 900, 600, "Breakout");
    ASSERT_NONNULL(window);

    RectangleI32 windowRect = WindowGetRectangle(window);
    // SetMouseVisibility(MOUSEVISIBILITYSTATE_HIDDEN);
    SetMousePosition(window, windowRect.width / 2, windowRect.height / 2);

    LARGE_INTEGER performanceCounterFrequency = {0};
    ASSERT_NONZERO(QueryPerformanceFrequency(&performanceCounterFrequency));

    LARGE_INTEGER lastCounter = {0};
    ASSERT_NONZERO(QueryPerformanceCounter(&lastCounter));

    // TODO(gr3yknigh1): Destroy shaders after they are linked [2024/09/15]
    GLShaderProgramID shader = 0;
    {
        GLShaderProgramLinkData shaderLinkData = {0};
        shaderLinkData.vertexShader =
            GLCompileShaderFromFile(&runtimeScratch, "P:\\gfs\\assets\\breakout\\basic.frag.glsl", GL_SHADER_TYPE_FRAG);
        shaderLinkData.fragmentShader =
            GLCompileShaderFromFile(&runtimeScratch, "P:\\gfs\\assets\\breakout\\basic.vert.glsl", GL_SHADER_TYPE_VERT);
        shader = GLLinkShaderProgram(&runtimeScratch, &shaderLinkData);
        ASSERT_NONZERO(shader);
    }

    Camera camera = Camera_Make(window, -1, 1, CAMERA_VIEW_MODE_ORTHOGONAL);

    f32 dt = 0.0f;
    u64 lastCycleCount = __rdtsc();

    bool isFirstMainloopIteration = true;
    i32 lastMouseXPosition = 0;
    i32 lastMouseYPosition = 0;

    DrawContext drawContext = DrawContext_MakeEx(&runtimeScratch, &camera, shader);

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

        // Camera_Rotate(&camera, mouseXOffset, mouseYOffset);
        // Camera_HandleInput(&camera);

        DrawBegin(&drawContext);
        DrawClear(&drawContext, 0, 0, 0);

        Color4RGBA whiteColor = EMPTY_STRUCT(Color4RGBA);
        whiteColor.r = 1;
        whiteColor.g = 1;
        whiteColor.b = 1;
        DrawRectangle(&drawContext, 0, 0, 500, 500, 1, 0, whiteColor);

        DrawEnd(&drawContext);

        WindowUpdate(window);

        ///< Perfomance
        {
            u64 endCycleCount = __rdtsc();

            LARGE_INTEGER endCounter;
            ASSERT_NONZERO(QueryPerformanceCounter(&endCounter));

            // TODO(ilya.a): Display counter [2025/11/08]

            u64 cyclesElapsed = endCycleCount - lastCycleCount;
            LONGLONG counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;

            dt = (f32)(1000LL * endCounter.QuadPart) / performanceCounterFrequency.QuadPart;

            u64 msPerFrame = (1000 * counterElapsed) / performanceCounterFrequency.QuadPart;
            u64 framesPerSeconds = performanceCounterFrequency.QuadPart / counterElapsed;
            u64 megaCyclesPerFrame = cyclesElapsed / (1000 * 1000);

            char8 printBuffer[KILOBYTES(1)];
            sprintf(
                printBuffer,
                "%llums/f | %lluf/s | %llumc/f || mouse x=%d y=%d || camera yaw=%.3f pitch=%.3f || mouseOffset=[%.3f "
                "%.3f] || lastPos=[%d %d]\n",
                msPerFrame, framesPerSeconds, megaCyclesPerFrame, mousePosition.x, mousePosition.y, camera.yaw,
                camera.pitch, mouseXOffset, mouseYOffset, lastMouseXPosition, lastMouseYPosition);
            OutputDebugString(printBuffer);
            lastCounter = endCounter;
            lastCycleCount = endCycleCount;
        }

        if (isFirstMainloopIteration) {
            isFirstMainloopIteration = false;
        }
    }

    WindowClose(window);
    ScratchDestroy(&runtimeScratch);
}
