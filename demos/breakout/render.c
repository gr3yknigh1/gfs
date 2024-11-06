/*
 * FILE      demos\breakout\render.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "render.h"

#include <ft2build.h>
#include FT_FREETYPE_H // WTF?

#include <glad/glad.h>

#include <cglm/cglm.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>

#include <gfs/macros.h>
#include <gfs/types.h>
#include <gfs/assert.h>

Camera
Camera_Make(Window *window, f32 near, f32 far, CameraViewMode viewMode)
{
    Camera camera = EMPTY_STRUCT(Camera);

    camera.position[0] = 0;
    camera.position[1] = 0;
    camera.position[2] = 3.0f;

    camera.front[0] = 0;
    camera.front[1] = 0;
    camera.front[2] = -1.0f;

    camera.up[0] = 0;
    camera.up[1] = 1.0f;
    camera.up[2] = 0;

    camera.yaw = -90.0f;

    camera.pitch = 0.0f;
    camera.speed = 0.05f;

    camera.sensitivity = 0.1f;
    camera.fov = 45.0f;

    camera.near = near;
    camera.far = far;

    camera.viewMode = viewMode;
    camera.window = window;
    return camera;
}

void
Camera_Rotate(Camera *camera, f32 xOffset, f32 yOffset)
{
    xOffset *= camera->sensitivity;
    yOffset *= camera->sensitivity;

    camera->yaw += xOffset;
    camera->pitch += yOffset;

    camera->pitch = glm_clamp(camera->pitch, -89.0f, 89.0f);

    vec3 direction = {0};

    f32 yawRad = glm_rad(camera->yaw);
    f32 pitchRad = glm_rad(camera->pitch);
    direction[0] = cosf(yawRad) * cosf(pitchRad);
    direction[1] = sinf(pitchRad);
    direction[2] = sinf(yawRad) * cosf(pitchRad);

    glm_normalize_to(direction, camera->front);
}

void
Camera_HandleInput(Camera *camera)
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

void
Camera_GetProjectionMatix(Camera *camera, mat4 *projection)
{
    RectangleI32 windowRect = WindowGetRectangle(camera->window);

    if (camera->viewMode == CAMERA_VIEW_MODE_PERSPECTIVE) {
        glm_perspective(
            glm_rad(camera->fov),
            (f32)windowRect.width / (f32)windowRect.height,
            //                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
            // NOTE(gr3yknigh1): Viewport width and height (not the window).
            // [2024/09/22]
            camera->near, camera->far, *projection);
    } else if (camera->viewMode == CAMERA_VIEW_MODE_ORTHOGONAL) {
        glm_ortho(
            0, (f32)windowRect.width, 0, (f32)windowRect.height, camera->near,
            camera->far, *projection);
    } else {
        ASSERT_ISTRUE(false);
    }
}

static FT_Library gFreeTypeLibrary;
static bool gFreeTypeLibraryWasInitialized = false;

Font *
Font_Make(cstring8 fontPath, u32 pixelWidth, u32 pixelHeight)
{
    if (!gFreeTypeLibraryWasInitialized) {
        gFreeTypeLibrary = EMPTY_STRUCT(FT_Library);
        ASSERT_ISOK(FT_Init_FreeType(&gFreeTypeLibrary));

        gFreeTypeLibraryWasInitialized = true;
    }

    FT_Face face = EMPTY_STRUCT(FT_Face);
    ASSERT_ISOK(FT_New_Face(gFreeTypeLibrary, fontPath, 0, &face));
    ASSERT_ISOK(FT_Set_Pixel_Sizes(face, pixelWidth, pixelHeight));

    static const char8 firstASCIIAlpha = 'A';
    static const char8 lastASCIIAlpha = 'z';

    Font *font = malloc(sizeof(Font));
    font->glyphsCount = lastASCIIAlpha - firstASCIIAlpha; // XXX
    font->glyphs = calloc(font->glyphsCount, sizeof(Glyph));

    font->pixelWidth = pixelWidth;
    font->pixelHeight = pixelHeight;

    // --- FreeType character loading ---
    Glyph *glyphWriteCursor = font->glyphs;

    glPixelStorei(
        GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction.

    for (char8 c = firstASCIIAlpha; c < lastASCIIAlpha; ++c) {
        ASSERT_ISOK(FT_Load_Char(face, c, FT_LOAD_RENDER));

        GL_CALL(glGenTextures(1, &glyphWriteCursor->texture));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, glyphWriteCursor->texture));

        GL_CALL(glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,
            face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer));

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glyphWriteCursor->size.x = face->glyph->bitmap.width;
        glyphWriteCursor->size.y = face->glyph->bitmap.rows;
        glyphWriteCursor->bearing.x = face->glyph->bitmap_left;
        glyphWriteCursor->bearing.y = face->glyph->bitmap_top;
        glyphWriteCursor->xAdvance = face->glyph->advance.x;

        ++glyphWriteCursor;
    }

    FT_Done_Face(face);

    return font;
}

void
Font_Destroy(Font *font)
{
    if (font->glyphs != NULL) {
        free(font->glyphs);
        font->glyphs = NULL;
    }

    free(font);
}

typedef struct {
    f32 position[2];
    f32 color[3];
} Vertex;

// static const Vertex RECTANGLE_VERTEXES[] = {
//     { { 1, 1 }, { 1, 1, 1 } }, // top-right
//     { { 1, 0 }, { 1, 1, 1 } }, // bottom-right
//     { { 0, 0 }, { 1, 1, 1 } }, // bottom-left
//     { { 0, 1 }, { 1, 1, 1 } }, // top-left
// };

static void
GenerateRectangleVertexes(
    Vertex *vertexes, f32 x, f32 y, f32 width, f32 height, Color3RGB color)
{
    // top-right
    vertexes[0].position[0] = x + width;
    vertexes[0].position[1] = y + height;

    vertexes[0].color[0] = color.r;
    vertexes[0].color[1] = color.g;
    vertexes[0].color[2] = color.b;

    // bottom-right
    vertexes[1].position[0] = x + width;
    vertexes[1].position[1] = y;

    vertexes[1].color[0] = color.r;
    vertexes[1].color[1] = color.g;
    vertexes[1].color[2] = color.b;

    // bottom-left
    vertexes[2].position[0] = x;
    vertexes[2].position[1] = y;

    vertexes[2].color[0] = color.r;
    vertexes[2].color[1] = color.g;
    vertexes[2].color[2] = color.b;

    // top-left
    vertexes[3].position[0] = x;
    vertexes[3].position[1] = y + height;

    vertexes[3].color[0] = color.r;
    vertexes[3].color[1] = color.g;
    vertexes[3].color[2] = color.b;
}

#if 0
static const u32 RECTANGLE_INDICIES[] = {0, 1, 2, 0, 2, 3};
#endif

DrawContext
DrawContext_MakeEx(
    Scratch *scratch, Camera *camera, GLShaderProgramID rectangleShader,
    GLShaderProgramID textShader)
{
    DrawContext context = EMPTY_STRUCT(DrawContext);

    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    context.camera = camera;

    // ---- Rect rendering ---- //

    context.rectDrawInfo.shader = rectangleShader;
    context.rectDrawInfo.uniformLocationModel =
        GLShaderFindUniformLocation(rectangleShader, "u_Model");
    context.rectDrawInfo.uniformLocationProjection =
        GLShaderFindUniformLocation(rectangleShader, "u_Projection");

    context.rectDrawInfo.va = GLVertexArrayMake();
    context.rectDrawInfo.vb = GLVertexBufferMake(NULL, 0);
    context.rectDrawInfo.eb = GLElementBufferMake(NULL, 0);
    context.rectDrawInfo.layout = GLVertexBufferLayoutMake(scratch);
    GLVertexBufferLayoutPushAttributeF32(&context.rectDrawInfo.layout, 2);
    GLVertexBufferLayoutPushAttributeF32(&context.rectDrawInfo.layout, 3);
    GLVertexArrayAddBuffer(
        context.rectDrawInfo.va, &context.rectDrawInfo.vb,
        &context.rectDrawInfo.layout);

    // ---- Text rendering ----

    // --- Setup OpenGL buffers for characters ---

    context.textDrawInfo.shader = textShader;

    context.textDrawInfo.va = GLVertexArrayMake();
    context.textDrawInfo.vb = GLVertexBufferMake(NULL, sizeof(f32) * 6 * 4);
    //                      /   /
    // ____________________/___/ [2024/10/30]
    // NOTE(gr3yknigh1): 6 - Count of vertexes, 4 - size of vertex

    context.textDrawInfo.layout = GLVertexBufferLayoutMake(scratch);
    GLVertexBufferLayoutPushAttributeF32(&context.textDrawInfo.layout, 2);
    GLVertexBufferLayoutPushAttributeF32(&context.textDrawInfo.layout, 2);

    GLVertexArrayAddBuffer(
        context.textDrawInfo.va, &context.textDrawInfo.vb,
        &context.textDrawInfo.layout);

    // TODO(gr3yknigh1): Destroy shaders after they are linked [2024/09/15]

    GL_CALL(glUseProgram(context.textDrawInfo.shader));

    context.textDrawInfo.uniformLocationColor =
        GLShaderFindUniformLocation(textShader, "u_Color");
    context.textDrawInfo.uniformLocationTexture =
        GLShaderFindUniformLocation(textShader, "u_Texture");
    context.textDrawInfo.uniformLocationProjection =
        GLShaderFindUniformLocation(textShader, "u_Projection");

    GLShaderSetUniformV3F32(
        textShader, context.textDrawInfo.uniformLocationColor, 1, 1, 1);
    GLShaderSetUniformI32(
        textShader, context.textDrawInfo.uniformLocationTexture, 0);

    return context;
}

void
DrawContext_SelectFont(DrawContext *context, Font *font)
{
    context->textDrawInfo.selectedFont = font;
}

void
DrawBegin(DrawContext *ctx)
{
    glm_mat4_copy(GLM_MAT4_IDENTITY, ctx->model);
    Camera_GetProjectionMatix(ctx->camera, &ctx->projection);

    GLShaderProgramID rectShader = ctx->rectDrawInfo.shader;
    GL_CALL(glUseProgram(rectShader));

    GLShaderSetUniformM4F32(
        rectShader, ctx->rectDrawInfo.uniformLocationModel,
        (f32 *)ctx->model);
    GLShaderSetUniformM4F32(
        rectShader, ctx->rectDrawInfo.uniformLocationProjection,
        (f32 *)ctx->projection);

    GLShaderProgramID textShader = ctx->textDrawInfo.shader;
    GL_CALL(glUseProgram(textShader));

    GLShaderSetUniformM4F32(
        textShader, ctx->textDrawInfo.uniformLocationProjection,
        (f32 *)ctx->projection);
}

void
DrawClear(DrawContext *context, f32 r, f32 g, f32 b)
{
    UNUSED(context);

    GLClearEx(r, g, b, 1, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
DrawRectangle(
    DrawContext *ctx, f32 x, f32 y, f32 width, f32 height, f32 scale,
    f32 rotate, Color4RGBA color)
{
    UNUSED(scale);
    UNUSED(rotate);

    GLShaderProgramID shader = ctx->rectDrawInfo.shader;
    GL_CALL(glBindVertexArray(ctx->rectDrawInfo.va));
    GL_CALL(glUseProgram(shader));

    GLShaderSetUniformM4F32(
        shader, ctx->rectDrawInfo.uniformLocationProjection,
        (f32 *)ctx->projection);

    // --- Generate vertexes ---
    Vertex vertexes[4] = EMPTY_STACK_ARRAY;
    Color3RGB rectangleColor = {color.r, color.g, color.b};
    GenerateRectangleVertexes(vertexes, x, y, width, height, rectangleColor);
    static const u32 indicies[] = {0, 1, 2, 0, 2, 3};

    // --- Sending geometry ---
    GLVertexBufferSendData(
        &ctx->rectDrawInfo.vb, vertexes, sizeof(Vertex) * 4);
    GLElementBufferSendData(
        &ctx->rectDrawInfo.eb, indicies, STATIC_ARRAY_LENGTH(indicies));

    GLDrawElements(
        &ctx->rectDrawInfo.eb, &ctx->rectDrawInfo.vb,
        ctx->rectDrawInfo.va);
}

void
DrawString(
    DrawContext *ctx, f32 x, f32 y, cstring8 s, f32 scale, Color4RGBA color)
{
    GLShaderProgramID shader = ctx->textDrawInfo.shader;

    GL_CALL(glUseProgram(shader));

    GL_CALL(glActiveTexture(GL_TEXTURE0));
    GL_CALL(glBindVertexArray(ctx->textDrawInfo.va));

    GLShaderSetUniformM4F32(
        shader, ctx->textDrawInfo.uniformLocationProjection,
        (f32 *)ctx->projection);

    const char8 *sCursor = s;

    Font *font = ctx->textDrawInfo.selectedFont;

    while (*sCursor) {
        if (*sCursor == ' ') {
            static const f32 SPACE_SIZE = 20;
            x += SPACE_SIZE; // XXX
            ++sCursor;
            continue;
        }

        if (*sCursor < 'A' || *sCursor > 'z') {
            ++sCursor;
            continue;
        }

        Glyph *glyph = font->glyphs + (*sCursor - 'A');

        f32 xpos = x + glyph->bearing.x * scale;
        f32 ypos = y - (glyph->size.y - glyph->bearing.y) * scale;

        f32 w = glyph->size.x * scale;
        f32 h = glyph->size.y * scale;
        // update VBO for each glyph
        f32 vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},    {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f},    {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};
        // render glyph texture over quad
        GL_CALL(glBindTexture(GL_TEXTURE_2D, glyph->texture));
        // update content of VBO memory

        GLVertexBufferSendData(
            &ctx->textDrawInfo.vb, vertices, sizeof(vertices));
        GLDrawTriangles(
            &ctx->textDrawInfo.vb, &ctx->textDrawInfo.layout,
            ctx->textDrawInfo.va);

        // now advance cursors for next glyph (note that advance is
        // number of 1/64 pixels)
        x += (glyph->xAdvance >> 6) * scale;
        // bitshift by 6 to get value in pixels (2^6 = 64)

        ++sCursor;
    }
}

void
DrawEnd(DrawContext *context)
{
    UNUSED(context);
}
