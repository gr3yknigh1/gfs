/*
 * Badcraft. Minecraft clone.
 *
 * References:
 *     - https://sibras.github.io/OpenGL4-Tutorials/docs/Tutorials/01-Tutorial1/#part-1-getting-required-libraries
 *
 * TODOs:
 *   - [X] Render with element buffer
 *   - [ ] Learn and possibly implement instancing rendering
 *   - [X] Implement basic chunk render system
 *   - [ ] Try to use strip rendering
 *   - [ ] Learn and implement texture atlas
 *
 * FILE      demos\badcraft\main.cpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 */
#include <cstdio>
#include <cmath>

#include <SDL2/SDL.h>
#include <glad/glad.h>

#include <glm/glm.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <gfs/atlas.h>
#include <gfs/random.h>
#include <gfs/macros.h>
#include <gfs/assert.h>
#include <gfs/game_state.h>
#include <gfs/render_opengl.h>
#include <gfs/physics.h>

#include "camera.hpp"

#define BLOCK_SIDE_SIZE EXPAND(1)
#define CHUNK_SIDE_SIZE EXPAND(16)
#define CHUNK_MAX_BLOCK_COUNT EXPAND(CHUNK_SIDE_SIZE *CHUNK_SIDE_SIZE *CHUNK_SIDE_SIZE)
#define FACE_PER_BLOCK EXPAND(6)
#define INDEXES_PER_FACE EXPAND(6)
#define VERTEXES_PER_FACE EXPAND(4)

#define WORLD_CHUNK_X_COUNT EXPAND(1)
#define WORLD_CHUNK_Y_COUNT EXPAND(1)
#define WORLD_CHUNK_Z_COUNT EXPAND(1)
#define WORLD_CHUNK_COUNT EXPAND(WORLD_CHUNK_X_COUNT * WORLD_CHUNK_Y_COUNT * WORLD_CHUNK_Z_COUNT)

#define BLOCK_MIN_X 0
#define BLOCK_MIN_Y 0
#define BLOCK_MIN_Z 0

#define BLOCK_MAX_X WORLD_CHUNK_X_COUNT * CHUNK_SIDE_SIZE
#define BLOCK_MAX_Y WORLD_CHUNK_Y_COUNT * CHUNK_SIDE_SIZE
#define BLOCK_MAX_Z WORLD_CHUNK_Z_COUNT * CHUNK_SIDE_SIZE


enum class BlockType : u16 {
    Nothing,
    Stone,
};

typedef struct {
    BlockType type;
} Block;

typedef struct {
    f32 position[3];
    f32 color[3];
    f32 uv[2];
} Vertex;

typedef struct {
    Vertex vertexes[VERTEXES_PER_FACE];  // 4
} Face;

enum class ChunkState : u32 {
    NotTouched = 0,
    TerrainGenerated,
    TerrainUnloaded,
    GeometryGenerated,
    GeometryUnloaded,
    Dirty,
};

#define ARRAY(TYPE) \
    struct { \
        TYPE *data; \
        u64 capacity; \
        u64 count; \
    }

typedef struct {
    Block blocks[CHUNK_MAX_BLOCK_COUNT];

    ARRAY(Face) faces;
    ARRAY(u32) indexes;

    Vector3F32 coords;
    ChunkState state;
} Chunk;


typedef struct {
    ARRAY(Chunk) chunks;
} World;

const static Face FRONT_FACE = LITERAL(Face){{
    // Position  Colors      UV
    {{0, 1, 1}, {1, 1, 1}, {0, 1}}, // [00] top-left
    {{1, 1, 1}, {1, 1, 1}, {1, 1}}, // [01] top-right
    {{1, 0, 1}, {1, 1, 1}, {1, 0}}, // [02] bottom-right
    {{0, 0, 1}, {1, 1, 1}, {0, 0}}  // [03] bottom-left
}};

const static u32 FRONT_FACE_INDEXES[6] = {
    0, 1, 3, // top-left
    3, 1, 2, // bottom-right
};

const static Face BACK_FACE = LITERAL(Face){{
    // Position  Colors     UV
    {{1, 1, 0}, {1, 1, 1}, {0, 1}}, // [0] [04] top-left
    {{0, 1, 0}, {1, 1, 1}, {1, 1}}, // [1] [05] top-right
    {{0, 0, 0}, {1, 1, 1}, {1, 0}}, // [2] [06] bottom-right
    {{1, 0, 0}, {1, 1, 1}, {0, 0}}  // [3] [07] bottom-left
}};

const static u32 BACK_FACE_INDEXES[6] = {
    0, 1, 2, // 4, 5, 6, // top-left
    2, 3, 0, // 6, 7, 4, // bottom-right
};

const static Face TOP_FACE = LITERAL(Face){{
    // Position  Colors     UV
    {{0, 1, 0}, {1, 1, 1}, {0, 1}}, // [0] [08] top-left
    {{1, 1, 0}, {1, 1, 1}, {1, 1}}, // [1] [09] top-right
    {{1, 1, 1}, {1, 1, 1}, {1, 0}}, // [2] [10] bottom-right
    {{0, 1, 1}, {1, 1, 1}, {0, 0}}  // [3] [11] bottom-left
}};

const static u32 TOP_FACE_INDEXES[6] = {
    3, 0, 1, // 11, 8,  9,  // top-left
    1, 2, 3, // 9,  10, 11, // bottom-right
};

const static Face BOTTOM_FACE = LITERAL(Face){{
    // Position  Colors    UV
    {{0, 0, 1}, {1, 1, 1}, {0, 1}}, // [0] [12] top-left
    {{1, 0, 1}, {1, 1, 1}, {1, 1}}, // [1] [13] top-right
    {{1, 0, 0}, {1, 1, 1}, {1, 0}}, // [2] [14] bottom-right
    {{0, 0, 0}, {1, 1, 1}, {0, 0}}  // [3] [15] bottom-left
}};

const static u32 BOTTOM_FACE_INDEXES[6] = {
    0, 1, 3, // 12, 13, 15, // top-left
    3, 1, 2, // 15, 13, 14, // bottom-right
};

const static Face LEFT_FACE = LITERAL(Face){{
    // Position  Colors      UV
    {{0, 1, 0}, {1, 1, 1}, {0, 1}}, // [0] [16] top-left
    {{0, 1, 1}, {1, 1, 1}, {1, 1}}, // [1] [17] top-right
    {{0, 0, 1}, {1, 1, 1}, {0, 0}}, // [2] [18] bottom-right
    {{0, 0, 0}, {1, 1, 1}, {1, 0}}  // [3] [19] bottom-left
}};

const static u32 LEFT_FACE_INDEXES[6] = {
    3, 0, 1, // 19, 16, 17, // top-left
    1, 2, 3, // 17, 18, 19  // bottom-right
};

const static Face RIGHT_FACE = LITERAL(Face){{
    // Position  Colors      UV
    {{1, 1, 1}, {1, 1, 1}, {0, 1}}, // [0] [20] top-left
    {{1, 1, 0}, {1, 1, 1}, {1, 1}}, // [1] [21] top-right
    {{1, 0, 0}, {1, 1, 1}, {0, 0}}, // [2] [22] bottom-right
    {{1, 0, 1}, {1, 1, 1}, {1, 0}}  // [3] [23] bottom-left
}};

const static u32 RIGHT_FACE_INDEXES[6] = {
    0, 1, 3, // 20, 21, 23, // top-left
    3, 1, 2, // 23, 21, 22  // bottom-right
};

static Chunk ChunkMake(Scratch *scratch, f32 x, f32 y, f32 z);
static void ChunkGenerateBlocks(World *world, Chunk *chunk);
static void ChunkGenerateGeometry(World *world, Chunk *chunk, Atlas *atlas);

static void MoveFaces(Face *faces, u32 faceCount, f32 x, f32 y, f32 z);
static void CameraHandleInput(Camera *camera, f32 deltaTime);
static BlockType GenerateNextBlock(f32 x, f32 y, f32 z);

static void AssignTextures(Face *faces, u32 faceCount, Atlas *atlas, BlockType blockType);

static const u8 *gSDLKeyState = NULL;


int
main(int argc, char *args[]) {
    UNUSED(argc);
    UNUSED(args);

    Scratch runtimeScratch = ScratchMake(GIGABYTES(1));

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

    const byte *glVendor = glGetString(GL_VENDOR);
    const byte *glRenderer = glGetString(GL_RENDERER);
    const byte *glVersion = glGetString(GL_VERSION);
    const byte *glExtensions = glGetString(GL_EXTENSIONS);
    const byte *glShaderLanguage = glGetString(GL_SHADING_LANGUAGE_VERSION);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Vendor: %s\n", glVendor);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Renderer: %s, v%s\n", glRenderer, glVersion);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Extentions: %s\n", glExtensions);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Language: %s\n", glShaderLanguage);

    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL_CALL(glEnable(GL_DEPTH_TEST));

    // GL_CALL(glEnable(GL_CULL_FACE));
    // GL_CALL(glCullFace(GL_FRONT));
    // GL_CALL(glFrontFace(GL_CCW));

    GLShaderProgramLinkData shaderLinkData = INIT_EMPTY_STRUCT(GLShaderProgramLinkData);
    shaderLinkData.vertexShader =
        GLCompileShaderFromFile(&runtimeScratch, "assets/basic.frag.glsl", GL_SHADER_TYPE_FRAG);
    shaderLinkData.fragmentShader =
        GLCompileShaderFromFile(&runtimeScratch, "assets/basic.vert.glsl", GL_SHADER_TYPE_VERT);
    GLShaderProgramID shader = GLLinkShaderProgram(&runtimeScratch, &shaderLinkData);
    ASSERT_NONZERO(shader);

    Atlas atlas = AtlasFromFile(&runtimeScratch, "assets/atlas.bmp", 16, 16, COLOR_LAYOUT_BGRA);

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
    GL_CALL(glBindTexture(GL_TEXTURE_2D, atlas.texture));

    World world = INIT_EMPTY_STRUCT(World);
    world.chunks.capacity = WORLD_CHUNK_COUNT;
    world.chunks.count = 0;
    world.chunks.data = static_cast<Chunk *>(ScratchAllocZero(&runtimeScratch, sizeof(Chunk) * world.chunks.capacity));

    for (u32 chunkIndex = 0; chunkIndex < WORLD_CHUNK_COUNT; ++chunkIndex) {
        Chunk *chunk = world.chunks.data + chunkIndex;

        Vector3U32 chunkCoords = GetCoordsFrom3DGridArrayOffsetRM(WORLD_CHUNK_X_COUNT, WORLD_CHUNK_Y_COUNT, WORLD_CHUNK_Z_COUNT, chunkIndex);
        *chunk = ChunkMake(&runtimeScratch, chunkCoords.x, chunkCoords.y, chunkCoords.z);
        ChunkGenerateBlocks(&world, chunk);
        ChunkGenerateGeometry(&world, chunk, &atlas);

        ++world.chunks.count;
    }

    GLVertexArray chunkVertexArray = GLVertexArrayMake();
    GLVertexBuffer chunkVertexBuffer = GLVertexBufferMake(NULL, VERTEXES_PER_FACE * FACE_PER_BLOCK * CHUNK_MAX_BLOCK_COUNT);
    GLVertexBufferLayout chunkVertexBufferLayout = GLVertexBufferLayoutMake(&runtimeScratch);  // XXX
    GLVertexBufferLayoutPushAttributeF32(&chunkVertexBufferLayout, 3);
    GLVertexBufferLayoutPushAttributeF32(&chunkVertexBufferLayout, 3);
    GLVertexBufferLayoutPushAttributeF32(&chunkVertexBufferLayout, 2);
    GLVertexArrayAddBuffer(chunkVertexArray, &chunkVertexBuffer, &chunkVertexBufferLayout);
    GLElementBuffer chunkElementBuffer = GLElementBufferMake(NULL, INDEXES_PER_FACE * FACE_PER_BLOCK * CHUNK_MAX_BLOCK_COUNT);

    bool showFrame = false;

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

        if (gSDLKeyState[SDL_SCANCODE_F]) {
            showFrame = !showFrame;

            if (showFrame) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
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

        glm::mat4 view = CameraGetViewMatix(&camera);
        glm::mat4 projection = CameraGetProjectionMatix(&camera, windowWidth, windowHeight);
        glm::mat4 model = glm::identity<glm::mat4>();

        GLShaderSetUniformM4F32(shader, uniformViewLocation, glm::value_ptr(view));
        GLShaderSetUniformM4F32(shader, uniformProjectionLocation, glm::value_ptr(projection));
        GLShaderSetUniformM4F32(shader, uniformModelLocation, glm::value_ptr(model));

        u32 faceCount = 0;
        u32 indexesCount = 0;
        u32 drawCalls = 0;

        for (u32 chunkIndex = 0; chunkIndex < WORLD_CHUNK_COUNT; ++chunkIndex) {
            Chunk *chunk = world.chunks.data + chunkIndex;

            if (chunk->state == ChunkState::Dirty) {
                ChunkGenerateGeometry(&world, chunk, &atlas);
            }

            GLVertexBufferSendData(&chunkVertexBuffer, reinterpret_cast<f32 *>(chunk->faces.data), chunk->faces.count * sizeof(Face));
            GLElementBufferSendData(&chunkElementBuffer, chunk->indexes.data, chunk->indexes.count);
            GLDrawElements(&chunkElementBuffer, &chunkVertexBuffer, chunkVertexArray);

            faceCount += chunk->faces.count;
            indexesCount += chunk->indexes.count;
            ++drawCalls;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Debug Information");
        ImGui::Text("FPS: %.05f", 1 / deltaTime);
        ImGui::Text("DeltaTime: %.05f", deltaTime);
        ImGui::Text("Faces count: %u", faceCount);
        ImGui::Text("Indexes count: %u", indexesCount);
        ImGui::Text("Draw calls: %u", drawCalls);
        ImGui::Text("Camera position: [%.3f %.3f %.3f]", camera.position.x, camera.position.y, camera.position.z);
        ImGui::Text("Mouse offset: [%.3f %.3f]", mouseXOffset, mouseYOffset);

        if (ImGui::CollapsingHeader("Camera Options")) {
            ImGui::InputFloat("Speed", &camera.speed, 0.01f, 500.0f, "%.3f");
            ImGui::InputFloat("Sensitivity", &camera.sensitivity, 0.01f, 500.0f, "%.3f");
            ImGui::InputFloat("FOV", &camera.fov, 20.0f, 180.0f, "%.3f");
            ImGui::InputFloat("Near plane", &camera.near, 0.01f, 10.0f, "%.3f");
            ImGui::InputFloat("Far plane", &camera.far, 20, 500.0f, "%.3f");
        }

        ImGui::End();

        ImGui::Render();

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

static inline void
MovePositionArray(f32 *position, f32 x, f32 y, f32 z) {
    position[0] += x;
    position[1] += y;
    position[2] += z;
}

static void
MoveFaces(Face *faces, u32 faceCount, f32 x, f32 y, f32 z) {
    for (u32 faceIndex = 0; faceIndex < faceCount; ++faceIndex) {
        Face *face = faces + faceIndex;
        MovePositionArray(face->vertexes[0].position, x, y, z);
        MovePositionArray(face->vertexes[1].position, x, y, z);
        MovePositionArray(face->vertexes[2].position, x, y, z);
        MovePositionArray(face->vertexes[3].position, x, y, z);
    }
}

/*
 * Can't move this function from this file, inner
 * logic bound to global variable `gSDLKeyState`.
 */
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

static inline Vector2U32
ConvertBlockTypeToTileCoords(Atlas *atlas, BlockType blockType) {
    Vector2U32 coords = INIT_EMPTY_STRUCT(Vector2U32);

    if (blockType == BlockType::Stone) {
        coords.x = 1;
        coords.y = 0;
    } else {
        coords.x = 0;
        coords.y = 0;
    }

    return coords;
}

static void
AssignTextures(Face *faces, u32 faceCount, Atlas *atlas, BlockType blockType) {
    Vector2U32 tileCoords = ConvertBlockTypeToTileCoords(atlas, blockType);
    // ^^^^^^^^^^^^^^^^^^
    // TODO(gr3yknigh1): Implement. There might be different textures for different faces. Leaving this for now.
    // [2024/10/09]

    f32 tileUVWidth = AtlasGetTileUVWidth(atlas);
    f32 tileUVHeight = AtlasGetTileUVHeight(atlas);
    TexCoords texCoords = AtlasTileCoordsToUV(atlas, tileCoords);

    for (u32 faceIndex = 0; faceIndex < faceCount; ++faceIndex) {
        Face *face = faces + faceIndex;

        face->vertexes[0].uv[0] = texCoords.t + tileUVHeight;
        face->vertexes[0].uv[1] = texCoords.s + tileUVWidth;

        face->vertexes[1].uv[0] = texCoords.t + tileUVHeight;
        face->vertexes[1].uv[1] = texCoords.s + 0;

        face->vertexes[2].uv[0] = texCoords.t + 0;
        face->vertexes[2].uv[1] = texCoords.s + 0;

        face->vertexes[3].uv[0] = texCoords.t + 0;
        face->vertexes[3].uv[1] = texCoords.s + tileUVWidth;
    }
}

static Chunk
ChunkMake(Scratch *scratch, f32 x, f32 y, f32 z) {
    Chunk chunk = INIT_EMPTY_STRUCT(Chunk);

    chunk.faces.capacity = CHUNK_MAX_BLOCK_COUNT * FACE_PER_BLOCK;
    chunk.faces.data = static_cast<Face *>(ScratchAllocZero(scratch, chunk.faces.capacity * sizeof(Face)));
    chunk.faces.count = 0;

    chunk.indexes.capacity = CHUNK_MAX_BLOCK_COUNT * FACE_PER_BLOCK * INDEXES_PER_FACE;
    chunk.indexes.data = static_cast<u32 *>(ScratchAllocZero(scratch, chunk.indexes.capacity * sizeof(u32)));
    chunk.indexes.count = 0;

    chunk.coords.x = x;
    chunk.coords.y = y;
    chunk.coords.z = z;

    chunk.state = ChunkState::NotTouched;

    return chunk;
}

static void
ChunkGenerateBlocks(World *world, Chunk *chunk) {
    for (u16 blockIndex = 0; blockIndex < CHUNK_MAX_BLOCK_COUNT; ++blockIndex) {
        Vector3U32 blockRelativePosition =
            GetCoordsFrom3DGridArrayOffsetRM(CHUNK_SIDE_SIZE, CHUNK_SIDE_SIZE, CHUNK_SIDE_SIZE, blockIndex);

        Block *block = chunk->blocks + blockIndex;

        Vector3F32 blockWorldPosition = INIT_EMPTY_STRUCT(Vector3F32);
        blockWorldPosition.x = chunk->coords.x * CHUNK_SIDE_SIZE + blockRelativePosition.x;
        blockWorldPosition.y = chunk->coords.y * CHUNK_SIDE_SIZE + blockRelativePosition.y;
        blockWorldPosition.z = chunk->coords.z * CHUNK_SIDE_SIZE + blockRelativePosition.z;
        block->type = GenerateNextBlock(blockWorldPosition.x, blockWorldPosition.y, blockWorldPosition.z);
    }
    chunk->state = ChunkState::TerrainGenerated;
}


static inline bool
IsNothing(const World *world, f32 rx, f32 ry, f32 rz, f32 wx, f32 wy, f32 wz) {
    // return true;
    if (wx < BLOCK_MIN_X || wy < BLOCK_MIN_Y || wz < BLOCK_MIN_Z || wx >= BLOCK_MAX_X || wy >= BLOCK_MAX_Y || wz >= BLOCK_MAX_Z) {
        return true;
    }

    Vector3F32 chunkCoords;
    chunkCoords.x = wx / CHUNK_SIDE_SIZE;
    chunkCoords.y = wy / CHUNK_SIDE_SIZE;
    chunkCoords.z = wz / CHUNK_SIDE_SIZE;

    i32 chunkIndex = GetOffsetFromCoords3DGridArrayRM(
        WORLD_CHUNK_X_COUNT, WORLD_CHUNK_Y_COUNT, WORLD_CHUNK_Z_COUNT, chunkCoords.x, chunkCoords.y, chunkCoords.z);
    Chunk *chunk = world->chunks.data + chunkIndex;

    if (rx < 0) {
        rx = CHUNK_SIDE_SIZE + rx;
    }

    if (ry < 0) {
        ry = CHUNK_SIDE_SIZE + ry;
    }

    if (rz < 0) {
        rz = CHUNK_SIDE_SIZE + rz;
    }

    i32 blockIndex = GetOffsetFromCoords3DGridArrayRM(CHUNK_SIDE_SIZE, CHUNK_SIDE_SIZE, CHUNK_SIDE_SIZE, rx, ry, rz);

    Block *block = chunk->blocks + blockIndex;
    return block->type == BlockType::Nothing;
}


/*
 * @breaf Emits faces and indices into chunk's geometry buffer.
 * @param rp Block's relative position
 * @param wp Block's world position
 */
static u32
EmitGeometryToChunk(World *world, Chunk *chunk, const Vector3F32 *rp, const Vector3F32 *wp) {
    u32 cursor = 0;

    Face *facesCursor = chunk->faces.data + chunk->faces.count;
    u32 *indexesCursor = chunk->indexes.data + chunk->indexes.count;

    u32 indexes[INDEXES_PER_FACE] = {0};

    // Front
    if (IsNothing(world, rp->x + 0, rp->y + 0, rp->z + 1, wp->x + 0, wp->y + 0, wp->z + 1)) {
        facesCursor[cursor] = FRONT_FACE;

        MemoryCopy(indexes, FRONT_FACE_INDEXES, INDEXES_PER_FACE * sizeof(u32));
        for (u16 indexIndex = 0; indexIndex < INDEXES_PER_FACE; ++indexIndex) {
            indexes[indexIndex] += cursor * VERTEXES_PER_FACE;
        }

        MemoryCopy(indexesCursor + cursor * INDEXES_PER_FACE, indexes, INDEXES_PER_FACE * sizeof(u32));
        ++cursor;
    }

    // Back
    if (IsNothing(world, rp->x + 0, rp->y + 0, rp->z - 1, wp->x + 0, wp->y + 0, wp->z - 1)) {
        facesCursor[cursor] = BACK_FACE;

        MemoryCopy(indexes, BACK_FACE_INDEXES, INDEXES_PER_FACE * sizeof(u32));

        for (u16 indexIndex = 0; indexIndex < INDEXES_PER_FACE; ++indexIndex) {
            indexes[indexIndex] += cursor * VERTEXES_PER_FACE;
        }

        MemoryCopy(indexesCursor + cursor * INDEXES_PER_FACE, indexes, INDEXES_PER_FACE * sizeof(u32));
        ++cursor;
    }

    // Top
    if (IsNothing(world, rp->x + 0, rp->y + 1, rp->z + 0, wp->x + 0, wp->y + 1, wp->z + 0)) {
        facesCursor[cursor] = TOP_FACE;

        MemoryCopy(indexes, TOP_FACE_INDEXES, INDEXES_PER_FACE * sizeof(u32));

        for (u16 indexIndex = 0; indexIndex < INDEXES_PER_FACE; ++indexIndex) {
            indexes[indexIndex] += cursor * VERTEXES_PER_FACE;
        }

        MemoryCopy(indexesCursor + cursor * INDEXES_PER_FACE, indexes, INDEXES_PER_FACE * sizeof(u32));
        ++cursor;
    }

    // Bottom
    if (IsNothing(world, rp->x + 0, rp->y - 1, rp->z + 0, wp->x + 0, wp->y - 1, wp->z + 0)) {
        facesCursor[cursor] = BOTTOM_FACE;

        MemoryCopy(indexes, BOTTOM_FACE_INDEXES, INDEXES_PER_FACE * sizeof(u32));

        for (u16 indexIndex = 0; indexIndex < INDEXES_PER_FACE; ++indexIndex) {
            indexes[indexIndex] += cursor * VERTEXES_PER_FACE;
        }

        MemoryCopy(indexesCursor + cursor * INDEXES_PER_FACE, indexes, INDEXES_PER_FACE * sizeof(u32));
        ++cursor;
    }

    // Left
    if (IsNothing(world, rp->x - 1, rp->y + 0, rp->z + 0, wp->x - 1, wp->y + 0, wp->z + 0)) {
        facesCursor[cursor] = LEFT_FACE;

        u32 indexes[INDEXES_PER_FACE] = {0};
        MemoryCopy(indexes, LEFT_FACE_INDEXES, INDEXES_PER_FACE * sizeof(u32));

        for (u16 indexIndex = 0; indexIndex < INDEXES_PER_FACE; ++indexIndex) {
            indexes[indexIndex] += cursor * VERTEXES_PER_FACE;
        }

        MemoryCopy(indexesCursor + cursor * INDEXES_PER_FACE, indexes, INDEXES_PER_FACE * sizeof(u32));
        ++cursor;
    }

    // Right
    if (IsNothing(world, rp->x + 1, rp->y + 0, rp->z + 0, wp->x + 1, wp->y + 0, wp->z + 0)) {
        facesCursor[cursor] = RIGHT_FACE;

        u32 indexes[INDEXES_PER_FACE] = {0};
        MemoryCopy(indexes, RIGHT_FACE_INDEXES, INDEXES_PER_FACE * sizeof(u32));

        for (u16 indexIndex = 0; indexIndex < INDEXES_PER_FACE; ++indexIndex) {
            indexes[indexIndex] += cursor * VERTEXES_PER_FACE;
        }

        MemoryCopy(indexesCursor + cursor * INDEXES_PER_FACE, indexes, INDEXES_PER_FACE * sizeof(u32));
        ++cursor;
    }

    return cursor;
}

static inline Vector3F32
Vector3U32ConvertToVector3F32(const Vector3U32 *v) {
    Vector3F32 ret;
    ret.x = static_cast<f32>(v->x);
    ret.y = static_cast<f32>(v->y);
    ret.z = static_cast<f32>(v->z);
    return ret;
}

static void
ChunkGenerateGeometry(World *world, Chunk *chunk, Atlas *atlas) {
    chunk->faces.count = 0;
    chunk->indexes.count = 0;

    Block *blocks = chunk->blocks;
    u64 blocksCount = CHUNK_MAX_BLOCK_COUNT;

    for (u16 blockIndex = 0; blockIndex < blocksCount; ++blockIndex) {
        Block *block = blocks + blockIndex;

        if (block->type != BlockType::Nothing) {
            Vector3U32 blockRelativePositionInd =
                GetCoordsFrom3DGridArrayOffsetRM(CHUNK_SIDE_SIZE, CHUNK_SIDE_SIZE, CHUNK_SIDE_SIZE, blockIndex);
            Vector3F32 blockRelativePosition = Vector3U32ConvertToVector3F32(&blockRelativePositionInd);

            Vector3F32 blockWorldPosition = INIT_EMPTY_STRUCT(Vector3F32);
            blockWorldPosition.x = chunk->coords.x * CHUNK_SIDE_SIZE + blockRelativePositionInd.x;
            blockWorldPosition.y = chunk->coords.y * CHUNK_SIDE_SIZE + blockRelativePositionInd.y;
            blockWorldPosition.z = chunk->coords.z * CHUNK_SIDE_SIZE + blockRelativePositionInd.z;

            u32 facesEmmited = EmitGeometryToChunk(world, chunk, &blockRelativePosition, &blockWorldPosition);

            // Normalize geometry
            for (u32 indexIndex = 0; indexIndex < INDEXES_PER_FACE * facesEmmited; ++indexIndex) {
                (chunk->indexes.data + chunk->indexes.count)[indexIndex] += chunk->faces.count * VERTEXES_PER_FACE;
            }

            MoveFaces(chunk->faces.data + chunk->faces.count, facesEmmited, blockWorldPosition.x, blockWorldPosition.y, blockWorldPosition.z);
            AssignTextures(chunk->faces.data + chunk->faces.count, facesEmmited, atlas, block->type);
            chunk->faces.count += facesEmmited;
            chunk->indexes.count += facesEmmited * INDEXES_PER_FACE;
        }
    }

    chunk->state = ChunkState::GeometryGenerated;
}

static BlockType
GenerateNextBlock(f32 x, f32 y, f32 z) {
    static f32 frequency = 0.2;
    static f32 amplitude = 10;
    f32 surfaceMaxY = glm::sin(x * frequency) * amplitude + 20;

    if (static_cast<f32>(y) < surfaceMaxY) {
        return BlockType::Stone;
    }

    return BlockType::Nothing;
}
