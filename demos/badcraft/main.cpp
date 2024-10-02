/*
 * Badcraft. Minecraft clone.
 *
 * References:
 *     - https://sibras.github.io/OpenGL4-Tutorials/docs/Tutorials/01-Tutorial1/#part-1-getting-required-libraries
 *
 * FILE      demos\badcraft\main.cpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 */
#include <SDL_video.h>
#include <cstdio>

#include <SDL2/SDL.h>
#include <glad/glad.h>

#include <gfs/macros.h>
#include <gfs/assert.h>
#include <gfs/game_state.h>
#include <gfs/render_opengl.h>

int
main(int argc, char *args[]) {
    UNUSED(argc);
    UNUSED(args);

    Scratch runtimeScratch = ScratchMake(MEGABYTES(20));

    SDL_version v = INIT_EMPTY_STRUCT(SDL_version);
    SDL_GetVersion(&v);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL version %d.%d.%d\n", v.major, v.minor, v.patch);

    ASSERT_ISZERO(SDL_Init(SDL_INIT_EVERYTHING));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window *window =
        SDL_CreateWindow("Badcraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    ASSERT_NONNULL(window);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    ASSERT_NONNULL(context);
    ASSERT_NONZERO(gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress));

    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    GLShaderProgramLinkData shaderLinkData = {0};
    shaderLinkData.vertexShader =
        GLCompileShaderFromFile(&runtimeScratch, "demos\\badcraft\\basic.frag.glsl", GL_SHADER_TYPE_FRAG);
    shaderLinkData.fragmentShader =
        GLCompileShaderFromFile(&runtimeScratch, "demos\\badcraft\\basic.vert.glsl", GL_SHADER_TYPE_VERT);
    GLShaderProgramID shader = GLLinkShaderProgram(&runtimeScratch, &shaderLinkData);
    ASSERT_NONZERO(shader);

    static const f32 vertices[] = {
        // pos              // color
        +0.0f, +0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, //
        +0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f  //
    };

    GLVertexArray va = GLVertexArrayMake();
    GLVertexBuffer vb = GLVertexBufferMake(vertices, sizeof(vertices));

    GLVertexBufferLayout vbLayout = GLVertexBufferLayoutMake(&runtimeScratch);
    GLVertexBufferLayoutPushAttributeF32(&vbLayout, 3);
    GLVertexBufferLayoutPushAttributeF32(&vbLayout, 4);

    GLVertexArrayAddBuffer(va, &vb, &vbLayout);

    // Enable VSync
    SDL_GL_SetSwapInterval(-1);

    while (!GameStateShouldStop()) {
        SDL_Event event = INIT_EMPTY_STRUCT(SDL_Event);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                GameStateStop();
            }
        }

        i32 windowWidth = 0, windowHeight = 0;
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        glViewport(0, 0, windowWidth, windowHeight);

        GLClear(0, 0, 0, 1); // TODO: Map from 0..255 to 0..1
        GLDrawTriangles(&vb, &vbLayout, va, shader, 0);

        SDL_GL_SwapWindow(window);
    }

    SDL_Quit();

    return 0;
}
