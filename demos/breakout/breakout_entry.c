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

#include <ft2build.h>
#include FT_FREETYPE_H  // WTF?

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
#include <gfs/random.h>

#include "breakout_render.h"
#include "breakout_sound.h"

static void GenerateTileGrid(
    Vector2F32 *tilePositions,
    f32 gridXPosition, f32 gridYPosition,
    u32 gridWidth, u32 gridHeight,
    u32 tileWidth, u32 tileHeight,
    f32 tileXPadding, f32 tileYPadding);

typedef struct {
    LARGE_INTEGER performanceCounterFrequency;
} Clock;

typedef struct {
    GLTexture texture;
    Vector2F32 size;
    Vector2F32 bearing;
    u32 xAdvance;
} Character;

void
Clock_Initialize(Clock *clock)
{
    ASSERT_NONZERO(QueryPerformanceFrequency(&clock->performanceCounterFrequency));
}

//
// Reference: https://gamedev.net/forums/topic/687578-how-to-properly-get-delta-time-on-modern-hardware/5337741/
//
u64
Clock_GetMilliseconds(Clock *clock)
{
    LARGE_INTEGER currentCounter;
    ASSERT_NONZERO(QueryPerformanceCounter(&currentCounter));
    u64 result = (1000LL * currentCounter.QuadPart) / clock->performanceCounterFrequency.QuadPart;
    return result;
}

f32
Clock_GetSeconds(Clock *clock)
{
    LARGE_INTEGER currentCounter;
    ASSERT_NONZERO(QueryPerformanceCounter(&currentCounter));
    f32 result = ((f32)currentCounter.QuadPart) / (f32)clock->performanceCounterFrequency.QuadPart;
    return result;
}

static bool IsOverlapped(f32 aXPosition, f32 aYPosition, f32 aWidth, f32 aHeight, f32 bXPosition, f32 bYPosition, f32 bWidth, f32 bHeight);

static f32
GetSign(f32 x)
{
    f32 ret = (x >= 0 ? 1 : -1);
    return ret;
}

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

    Clock runtimeClock = EMPTY_STRUCT(Clock);
    Clock_Initialize(&runtimeClock);

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

    u64 lastCycleCount = __rdtsc();

    DrawContext drawContext = DrawContext_MakeEx(&runtimeScratch, &camera, shader);

    f32 ballWidth = 10;
    f32 ballHeight = 10;
    f32 ballBaseSpeed = 350;
    f32 ballXPosition = windowRect.width / 2 - ballWidth / 2;
    f32 ballYPosition = windowRect.height / 3 - ballHeight / 3;
    f32 ballXVelocity = 0;
    f32 ballYVelocity = (-1) * ballBaseSpeed;

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
    bool *tileIsDisabled = malloc(sizeof(bool) * gridTileCount);
    MemoryZero(tileIsDisabled, sizeof(bool) * gridTileCount);

    GenerateTileGrid(tilePositions, gridXPosition, gridYPosition, gridXTileCount, gridYTileCount, tileWidth, tileHeight, gridXPadding, gridYPadding);

    f32 playerXVelocity = 0;
    f32 playerYVelocity = 0;
    f32 playerWidth = 100;
    f32 playerHeight = 20;
    f32 playerXPosition = windowRect.width / 2 - playerWidth / 2;
    f32 playerYPosition = 30;
    f32 playerSpeed = 700;

    f32 playerPreviousXVelocity = 0; // NOTE: better name `direction`
    f32 playerPreviousYVelocity = 0;

    bool doRenderGridBackground = false;

    f32 startClockTime = Clock_GetSeconds(&runtimeClock);
    f32 deltaTime = 0;

    // --- FreeType library initialization ---
    FT_Library ft = EMPTY_STRUCT(FT_Library);
    ASSERT_ISOK(FT_Init_FreeType(&ft));

    FT_Face face = EMPTY_STRUCT(FT_Face);
    ASSERT_ISOK(FT_New_Face(ft, "P:\\gfs\\assets\\breakout\\IBMPlexMono.ttf", 0, &face));

    FT_Set_Pixel_Sizes(face, 0, 48);

    u16 charactersCount = 128;
    Character *characterTable = malloc(sizeof(Character) * charactersCount);

    // --- FreeType character loading ---
    {
        Character *characterTableWriteCursor = characterTable;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction.

        for (char8 c = 0; c < charactersCount; ++c) {
            ASSERT_ISOK(FT_Load_Char(face, c, FT_LOAD_RENDER));

            glGenTextures(1, &characterTableWriteCursor->texture);
            glBindTexture(GL_TEXTURE_2D, characterTableWriteCursor->texture);

            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            characterTableWriteCursor->size.x = face->glyph->bitmap.width;
            characterTableWriteCursor->size.y = face->glyph->bitmap.rows;
            characterTableWriteCursor->bearing.x = face->glyph->bitmap_left;
            characterTableWriteCursor->bearing.y = face->glyph->bitmap_top;
            characterTableWriteCursor->xAdvance = face->glyph->advance.x;

            ++characterTableWriteCursor;
        }
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // --- Setup OpenGL buffers for characters ---

    GLVertexArray textVertexArray = GLVertexArrayMake();
    GLVertexBuffer textVertexBuffer = GLVertexBufferMake(NULL, sizeof(float) * 6 * 4);
    //                                                                         /   /
    // NOTE(gr3yknigh1): 6 - Count of vertexes, 4 - size of vertex ___________/___/
    // [2024/10/30]

    GLVertexBufferLayout textVertexBufferLayout = GLVertexBufferLayoutMake(&runtimeScratch);
    GLVertexBufferLayoutPushAttributeF32(&textVertexBufferLayout, 2);
    GLVertexBufferLayoutPushAttributeF32(&textVertexBufferLayout, 2);

    while (!GameStateShouldStop()) {

        deltaTime = Clock_GetSeconds(&runtimeClock) - startClockTime;
        startClockTime = Clock_GetSeconds(&runtimeClock);

        PoolEvents(window);

        playerXVelocity = 0;
        playerYVelocity = 0;

        if (IsKeyDown(KEY_A)) {
            if (playerXVelocity != 0) {
                playerPreviousXVelocity = playerXVelocity;
            }
            playerXVelocity = (-1) * playerSpeed;
        }

        if (IsKeyDown(KEY_D)) {
            if (playerXVelocity != 0) {
                playerPreviousXVelocity = playerXVelocity;
            }
            playerXVelocity = (+1) * playerSpeed;
        }


        playerXPosition += playerXVelocity * deltaTime;
        playerYPosition += playerYVelocity * deltaTime;

        {
            //
            // Balls
            //
            ballXPosition += ballXVelocity * deltaTime;
            ballYPosition += ballYVelocity * deltaTime;

            if (!IsOverlapped(0, 0, windowRect.width, windowRect.height, ballXPosition, ballYPosition, ballWidth, ballHeight)) {
                f32 xVelocityMod = 1;
                f32 yVelocityMod = 1;

                if (ballXPosition + ballWidth >= windowRect.width || ballXPosition <= 0) {
                    xVelocityMod = -xVelocityMod;
                }

                if (ballYPosition + ballHeight >= windowRect.height || ballYPosition <= 0) {
                    yVelocityMod = -yVelocityMod;
                }

                ballXVelocity = ballBaseSpeed * xVelocityMod * GetSign(ballXVelocity);
                ballYVelocity = ballBaseSpeed * yVelocityMod * GetSign(ballYVelocity);
            }


            for (u32 xTileIndex = 0; xTileIndex < gridXTileCount; ++xTileIndex) {
                for (u32 yTileIndex = 0; yTileIndex < gridYTileCount; ++yTileIndex) {
                    u32 tileIndex = GetOffsetFromCoords2DGridArrayRM(gridXTileCount, xTileIndex, yTileIndex);

                    if (tileIsDisabled[tileIndex]) {
                        continue;
                    }

                    Vector2F32 *tilePosition = tilePositions + tileIndex;

                    if (IsOverlapped(tilePosition->x, tilePosition->y, tileWidth, tileHeight, ballXPosition, ballYPosition, ballWidth, ballHeight)) {
                        f32 xVelocityMod = 1;
                        f32 yVelocityMod = 1;

                        if (ballXPosition + ballWidth >= tilePosition->x + tileWidth || ballXPosition <= tilePosition->x) {
                            xVelocityMod = -xVelocityMod;
                        }

                        if (ballYPosition + ballHeight >= tilePosition->y + tileHeight || ballYPosition <= tilePosition->y) {
                            yVelocityMod = -yVelocityMod;
                        }

                        ballXVelocity = ballBaseSpeed * xVelocityMod * GetSign(ballXVelocity);
                        ballYVelocity = ballBaseSpeed * yVelocityMod * GetSign(ballYVelocity);

                        tileIsDisabled[tileIndex] = true;
                    }
                }
            }

            if (IsOverlapped(playerXPosition, playerYPosition, playerWidth, playerHeight, ballXPosition, ballYPosition, ballWidth, ballHeight)) {
                ballXVelocity = ballBaseSpeed * GetSign(playerXVelocity == 0 ? playerPreviousXVelocity : playerXVelocity);
                ballYVelocity = ballBaseSpeed;
            }
        }

        DrawBegin(&drawContext);
        DrawClear(&drawContext, 0, 0, 0);

        for (u32 xTileIndex = 0; xTileIndex < gridXTileCount; ++xTileIndex) {
            for (u32 yTileIndex = 0; yTileIndex < gridYTileCount; ++yTileIndex) {
                u32 tileIndex = GetOffsetFromCoords2DGridArrayRM(gridXTileCount, xTileIndex, yTileIndex);

                if (tileIsDisabled[tileIndex]) {
                    continue;
                }

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

            u64 msPerFrame = (1000 * counterElapsed) / performanceCounterFrequency.QuadPart;
            u64 framesPerSeconds = performanceCounterFrequency.QuadPart / counterElapsed;
            u64 megaCyclesPerFrame = cyclesElapsed / (1000 * 1000);

            char8 printBuffer[KILOBYTES(1)];
            sprintf(
                printBuffer,
                "%llums/f | %lluf/s | %llumc/f | dt: %f\n",
                msPerFrame, framesPerSeconds, megaCyclesPerFrame, deltaTime);
            OutputDebugString(printBuffer);
            lastCounter = endCounter;
            lastCycleCount = endCycleCount;
        }
    }

    WindowClose(window);
    ScratchDestroy(&runtimeScratch);

    free(characterTable);
    free(tilePositions);
    free(tileIsDisabled);
}

static bool
IsOverlapped(f32 aXPosition, f32 aYPosition, f32 aWidth, f32 aHeight, f32 bXPosition, f32 bYPosition, f32 bWidth, f32 bHeight)
{
    return (aXPosition + aWidth) >= bXPosition
        && (bXPosition + bWidth) >= aXPosition
        && (aYPosition + aHeight) >= bYPosition
        && (bYPosition + bHeight) >= aYPosition;
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