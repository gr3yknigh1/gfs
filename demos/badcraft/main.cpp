/*
 * Badcraft. Minecraft clone.
 *
 * References:
 *     - https://sibras.github.io/OpenGL4-Tutorials/docs/Tutorials/01-Tutorial1/#part-1-getting-required-libraries
 *
 * TODOs:
 *   - [X] Render with element buffer
 *   - [ ] Learn and possibly implement instancing rendering
 *   - [ ] Implement chunking system
 *   - [ ] Try to use strip rendering
 *   - [ ] Learn and implement texture atlas
 *
 * FILE      demos\badcraft\main.cpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 */
#include <cstdio>

#include <SDL2/SDL.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <gfs/macros.h>
#include <gfs/assert.h>
#include <gfs/game_state.h>
#include <gfs/render_opengl.h>

/*
 * @breaf Chunk size in one of dimentions
 */
#define CHUNK_SIZE 16
#define CHUNK_BLOCK_COUNT CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE

typedef struct {
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;

    f32 yaw;
    f32 pitch;

    f32 speed;
    f32 sensitivity;
    f32 fov;
} Camera;

enum class BlockType : u16 {
    Nothing,
    Stone,
};

typedef struct {
    BlockType type;
    Vector3 position;
} Block;

typedef struct {
    Block blocks[CHUNK_BLOCK_COUNT];

    struct {
        Vector3 *data;
        usize capacity;
        u64 count;
    } faceBuffer;
} Chunk;

static Camera CameraMake(void);
static void CameraRotate(Camera *camera, f32 xOffset, f32 yOffset);
static void CameraHandleInput(Camera *camera, f32 deltaTime);
static glm::mat4 CameraGetViewMatix(Camera *camera);
static glm::mat4 CameraGetProjectionMatix(Camera *camera, i32 viewportWidth, i32 viewportHeight);

static f32 ClampF32(f32 value, f32 min, f32 max);

static const u8 *gSDLKeyState = NULL;

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

    SDL_Window *window = SDL_CreateWindow(
        "Badcraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 900, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    ASSERT_NONNULL(window);

    SDL_MaximizeWindow(window);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    ASSERT_NONNULL(context);
    ASSERT_NONZERO(gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress));

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 330");

    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL_CALL(glEnable(GL_DEPTH_TEST));

    GL_CALL(glEnable(GL_CULL_FACE));
    GL_CALL(glCullFace(GL_FRONT));
    GL_CALL(glFrontFace(GL_CCW));

    GLShaderProgramLinkData shaderLinkData = INIT_EMPTY_STRUCT(GLShaderProgramLinkData);
    shaderLinkData.vertexShader =
        GLCompileShaderFromFile(&runtimeScratch, "P:\\gfs\\assets\\basic.frag.glsl", GL_SHADER_TYPE_FRAG);
    shaderLinkData.fragmentShader =
        GLCompileShaderFromFile(&runtimeScratch, "P:\\gfs\\assets\\basic.vert.glsl", GL_SHADER_TYPE_VERT);
    GLShaderProgramID shader = GLLinkShaderProgram(&runtimeScratch, &shaderLinkData);
    ASSERT_NONZERO(shader);

    const Mesh *cubeMesh = GLGetCubeMesh(&runtimeScratch, GL_COUNTER_CLOCK_WISE);

    BMPicture picture = INIT_EMPTY_STRUCT(BMPicture);
    ASSERT_ISOK(BMPictureLoadFromFile(&picture, &runtimeScratch, "P:\\gfs\\assets\\kitty.bmp"));
    GLTexture texture = GLTextureMakeFromBMPicture(&picture);

    GLUniformLocation uniformVertexModifierLocation = GLShaderFindUniformLocation(shader, "u_VertexModifier");
    GLUniformLocation uniformVertexOffsetLocation = GLShaderFindUniformLocation(shader, "u_VertexOffset");
    GLUniformLocation uniformTextureLocation = GLShaderFindUniformLocation(shader, "u_Texture");

    GLUniformLocation uniformModelLocation = GLShaderFindUniformLocation(shader, "u_Model");
    GLUniformLocation uniformViewLocation = GLShaderFindUniformLocation(shader, "u_View");
    GLUniformLocation uniformProjectionLocation = GLShaderFindUniformLocation(shader, "u_Projection");

    GLShaderSetUniformF32(shader, uniformVertexModifierLocation, 1.0f);
    GLShaderSetUniformV3F32(shader, uniformVertexOffsetLocation, 0.3f, 0.3f, 0.3f);
    GLShaderSetUniformI32(shader, uniformTextureLocation, 0);

    i32 windowWidth = 0, windowHeight = 0;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // Enable VSync
    SDL_GL_SetSwapInterval(-1);

    SDL_bool isRelativeMouseMode = SDL_TRUE;
    SDL_SetRelativeMouseMode(isRelativeMouseMode);

    // Wrap mouse inside window
    SDL_WarpMouseInWindow(window, windowWidth / 2, windowHeight / 2);

    Camera camera = CameraMake();

    u64 currentPerfCounter = SDL_GetPerformanceCounter(), previousPerfCounter = 0;

    bool isFirstMouseMotion = true;

    GL_CALL(glUseProgram(shader));
    GL_CALL(glActiveTexture(GL_TEXTURE0)); // TODO(gr3yknigh1): Investigate in multi
                                           // texture support. [2024/09/22]
    GL_CALL(glBindTexture(GL_TEXTURE_2D, texture));

    Chunk *chunk = static_cast<Chunk *>(ScratchAllocZero(&runtimeScratch, sizeof(Chunk) * 1));

#if 0
    for (u16 blockIndex = 0; blockIndex < CHUNK_BLOCK_COUNT; ++blockIndex) {
        Block *block = chunk->blocks + blockIndex;
        Vector3U32 blockPosition = GetCoordsFrom3DGridArrayOffset(/**/);

        if (blockPosition.y == 1) {
            block->type = BlockType::Stone;
        }
    }
#endif

    while (!GameStateShouldStop()) {
        previousPerfCounter = currentPerfCounter;
        currentPerfCounter = SDL_GetPerformanceCounter();

        // Might be multiplied by 1000?
        f32 deltaTime = static_cast<f32>(
            (currentPerfCounter - previousPerfCounter) / static_cast<f32>(SDL_GetPerformanceFrequency()));

        // Input
        SDL_Event event = INIT_EMPTY_STRUCT(SDL_Event);

        f32 mouseXOffset = 0;
        f32 mouseYOffset = 0;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT) {
                GameStateStop();
            } else if (event.type == SDL_MOUSEMOTION) {

                if (isFirstMouseMotion || !isRelativeMouseMode) {
                    mouseXOffset = 0;
                    mouseYOffset = 0;
                    isFirstMouseMotion = false;
                } else {
                    mouseXOffset = event.motion.xrel;
                    mouseYOffset = event.motion.yrel;
                }
            }
        }

        gSDLKeyState = SDL_GetKeyboardState(nullptr);

        // Quit...

        if (gSDLKeyState[SDL_SCANCODE_Q]) {
            GameStateStop();
        }

        // Toggle mouse...

        if (gSDLKeyState[SDL_SCANCODE_ESCAPE]) {
            isRelativeMouseMode = isRelativeMouseMode ? SDL_FALSE : SDL_TRUE;
            SDL_SetRelativeMouseMode(isRelativeMouseMode);
        }

        // Update

        CameraRotate(&camera, mouseXOffset, mouseYOffset);
        CameraHandleInput(&camera, deltaTime);

        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        GL_CALL(glViewport(0, 0, windowWidth, windowHeight));
        // Render

        GLClear(0, 0, 0, 1); // TODO: Map from 0..255 to 0..1

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Debug Information");
        ImGui::Text("FPS: %.05f", 1 / deltaTime);
        ImGui::Text("DeltaTime: %.05f", deltaTime);
        ImGui::Text("Camera position: [%.3f %.3f %.3f]", camera.position.x, camera.position.y, camera.position.z);
        ImGui::Text("Mouse offset: [%.3f %.3f]", mouseXOffset, mouseYOffset);

        static bool renderOneCube = true;
        ImGui::Checkbox("Render one cube?", &renderOneCube);

        ImGui::End();

        ImGui::Render();

        glm::mat4 view = CameraGetViewMatix(&camera);
        glm::mat4 projection = CameraGetProjectionMatix(&camera, windowWidth, windowHeight);

        GLShaderSetUniformM4F32(shader, uniformViewLocation, glm::value_ptr(view));
        GLShaderSetUniformM4F32(shader, uniformProjectionLocation, glm::value_ptr(projection));

        // glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(chunk->position.x, chunk->position.y, chunk->position.z));
        // GLShaderSetUniformM4F32(shader, uniformModelLocation, glm::value_ptr(model));

#if 1
        if (renderOneCube) {
            glm::mat4 model = glm::identity<glm::mat4>();
            GLShaderSetUniformM4F32(shader, uniformModelLocation, glm::value_ptr(model));
            GLDrawMesh(cubeMesh);
        } else {
            for (u8 x = 0; x < 16; ++x) {
                for (u8 y = 0; y < 16; ++y) {
                    for (u8 z = 0; z < 16; ++z) {
                        glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(x, y, z));
                        GLShaderSetUniformM4F32(shader, uniformModelLocation, glm::value_ptr(model));
                        GLDrawMesh(cubeMesh);
                    }
                }
            }
        }
#endif

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_Quit();
    ScratchDestroy(&runtimeScratch);

    return 0;
}

static Camera
CameraMake() {
    Camera camera = {
        .position = {0, 0, 3.0f},
        .front = {0, 0, -1.0f},
        .up = {0, 1.0f, 0},

        .yaw = -90.0f,
        .pitch = 0.0f,

        .speed = 10.0f,
        .sensitivity = 0.5f,
        .fov = 45.0f,
    };
    return camera;
}

static void
CameraRotate(Camera *camera, f32 xOffset, f32 yOffset) {
    xOffset *= camera->sensitivity;
    yOffset *= camera->sensitivity;

    camera->yaw += xOffset * 1;
    camera->pitch += yOffset * -1;

    camera->pitch = ClampF32(camera->pitch, -89.0f, 89.0f);

    f32 yawRad = glm::radians(camera->yaw);
    f32 pitchRad = glm::radians(camera->pitch);

    glm::vec3 direction = LITERAL(glm::vec3){
        cosf(yawRad) * cosf(pitchRad),                  // TODO(gr3yknigh1): Change to `glm::*` functions
        sinf(pitchRad),
        sinf(yawRad) * cosf(pitchRad),
    };
    camera->front = glm::normalize(direction);
}

static void
CameraHandleInput(Camera *camera, f32 deltaTime) {

    if (gSDLKeyState[SDL_SCANCODE_W]) {
        camera->position += camera->front * camera->speed * deltaTime;
    }

    if (gSDLKeyState[SDL_SCANCODE_S]) {
        camera->position -= camera->front * camera->speed * deltaTime;
    }

    if (gSDLKeyState[SDL_SCANCODE_A]) {
        glm::vec3 direction = glm::normalize(glm::cross(camera->front, camera->up));
        camera->position -= direction * camera->speed * deltaTime;
    }

    if (gSDLKeyState[SDL_SCANCODE_D]) {
        glm::vec3 direction = glm::normalize(glm::cross(camera->front, camera->up));
        camera->position += direction * camera->speed * deltaTime;
    }
}

static glm::mat4
CameraGetViewMatix(Camera *camera) {
    return glm::lookAt(camera->position, camera->position + camera->front, camera->up);
}

static glm::mat4
CameraGetProjectionMatix(Camera *camera, i32 viewportWidth, i32 viewportHeight) {
    return glm::perspective(
        glm::radians(camera->fov), static_cast<f32>(viewportWidth) / static_cast<f32>(viewportHeight), 0.1f, 100.0f);
}

static f32
ClampF32(f32 value, f32 min, f32 max) {
    if (value > max) {
        return max;
    }

    if (value < min) {
        return min;
    }

    return value;
}
