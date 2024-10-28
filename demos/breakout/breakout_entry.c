/*
 * Breakout Game.
 *
 * FILE      demos\breakout\breakout_entry.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include <stdlib.h> // malloc, free
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

static void GenerateTileGrid(
    Vector2F32 *tilePositions,
    f32 gridXPosition, f32 gridYPosition,
    u32 gridWidth, u32 gridHeight,
    u32 tileWidth, u32 tileHeight,
    f32 tileXPadding, f32 tileYPadding);

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

    f32 dt = 0.0f;  // XXX
    u64 lastCycleCount = __rdtsc();

    DrawContext drawContext = DrawContext_MakeEx(&runtimeScratch, &camera, shader);

    f32 ballWidth = 10;
    f32 ballHeight = 10;
    f32 ballXPosition = windowRect.width / 2 - ballWidth / 2;
    f32 ballYPosition = windowRect.height / 3 - ballHeight / 3;

    f32 tileWidth = 120;
    f32 tileHeight = 30;
    f32 gridXPadding = 5;
    f32 gridYPadding = 5;
    u32 gridXTileCount = 6, gridYTileCount = 3;
    u32 gridTileCount = gridXTileCount * gridYTileCount;
    f32 gridWidth = gridXTileCount * tileWidth + (gridXPadding * (gridXTileCount - 1));
    f32 gridHeight = gridYTileCount * tileHeight + (gridYPadding * (gridYTileCount - 1));
    f32 gridXPosition = windowRect.width / 2 - gridWidth / 2;
    f32 gridYPosition = (f32)windowRect.height / 2;
    Vector2F32 *tilePositions = malloc(sizeof(Vector2F32) * gridTileCount);

    GenerateTileGrid(tilePositions, gridXPosition, gridYPosition, gridXTileCount, gridYTileCount, tileWidth, tileHeight, gridXPadding, gridYPadding);

    f32 playerWidth = 100;
    f32 playerHeight = 20;
    f32 playerXPosition = windowRect.width / 2 - playerWidth / 2;
    f32 playerYPosition = 30;
    f32 playerSpeed = 3;

    bool doRenderGridBackground = false;

    while (!GameStateShouldStop()) {
        PoolEvents(window);

        if (IsKeyDown(KEY_A)) {
            playerXPosition -= playerSpeed;
        }

        if (IsKeyDown(KEY_D)) {
            playerXPosition += playerSpeed;
        }

        DrawBegin(&drawContext);
        DrawClear(&drawContext, 0, 0, 0);

        for (u32 xTileIndex = 0; xTileIndex < gridXTileCount; ++xTileIndex) {
            for (u32 yTileIndex = 0; yTileIndex < gridYTileCount; ++yTileIndex) {
                u32 tileIndex = GetOffsetFromCoords2DGridArrayRM(gridXTileCount, xTileIndex, yTileIndex);
                Vector2F32 *tilePosition = tilePositions + tileIndex;

                DrawRectangle(&drawContext, tilePosition->x, tilePosition->y, tileWidth, tileHeight, 1, 0, COLOR4RGBA_GREEN);
            }
        }

        if (doRenderGridBackground) DrawRectangle(&drawContext, gridXPosition, gridYPosition, gridWidth, gridHeight, 1, 0, COLOR4RGBA_BLUE);

        DrawRectangle(&drawContext, playerXPosition, playerYPosition, playerWidth, playerHeight, 1, 0, COLOR4RGBA_RED);

        DrawRectangle(&drawContext, ballXPosition, ballYPosition, ballWidth, ballHeight, 1, 0, COLOR4RGBA_WHITE);

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
                "%llums/f | %lluf/s | %llumc/f || camera yaw=%.3f pitch=%.3f\n",
                msPerFrame, framesPerSeconds, megaCyclesPerFrame, camera.yaw,
                camera.pitch);
            OutputDebugString(printBuffer);
            lastCounter = endCounter;
            lastCycleCount = endCycleCount;
        }
    }

    WindowClose(window);
    ScratchDestroy(&runtimeScratch);

    free(tilePositions);
}

static void
GenerateTileGrid(
    Vector2F32 *tilePositions,
    f32 gridXPosition, f32 gridYPosition,
    u32 xTileCount, u32 yTileCount,
    u32 tileWidth, u32 tileHeight,
    f32 tileXPadding, f32 tileYPadding)
{
    for (u32 xTileIndex = 0; xTileIndex < xTileCount; ++xTileIndex) {
        for (u32 yTileIndex = 0; yTileIndex < yTileCount; ++yTileIndex) {
            u32 tileIndex = GetOffsetFromCoords2DGridArrayRM(xTileCount, xTileIndex, yTileIndex);
            Vector2F32 *tilePosition = tilePositions + tileIndex;

            tilePosition->x = gridXPosition + xTileIndex * (tileWidth + tileXPadding);
            tilePosition->y = gridYPosition + yTileIndex * (tileHeight + tileYPadding);
        }
    }
}