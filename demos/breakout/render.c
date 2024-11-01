/*
 * FILE      demos\breakout\render.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "render.h"

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

static const u32 RECTANGLE_INDICIES[] = {0, 1, 2, 0, 2, 3};

DrawContext
DrawContext_MakeEx(Scratch *scratch, Camera *camera, GLShaderProgramID shader)
{
    DrawContext context = EMPTY_STRUCT(DrawContext);

    context.camera = camera;
    context.shader = shader;

    context.glInfo.uniformLocationModel =
        GLShaderFindUniformLocation(shader, "u_Model");
    context.glInfo.uniformLocationProjection =
        GLShaderFindUniformLocation(shader, "u_Projection");

    // static const f32 vertices[] = {
    //     // positions   // colors
    //     1,  1,         1.0f, 0.0f, 0.0f, // top right
    //     1,  0,         0.0f, 1.0f, 0.0f, // bottom right
    //     0,  0,         0.0f, 0.0f, 1.0f, // bottom left
    //     0,  1,         1.0f, 1.0f, 0.0f  // top left
    // };
    // static const u32 indicies[] = {0, 1, 2, 0, 2, 3};

    context.glInfo.va = GLVertexArrayMake();
    context.glInfo.vb = GLVertexBufferMake(
        NULL /* RECTANGLE_VERTEXES */, 0 /* sizeof(vertices) */);
    context.glInfo.eb = GLElementBufferMake(
        NULL /* indicies */, 0 /* STATIC_ARRAY_LENGTH(indicies) */);

    context.glInfo.vbLayout = GLVertexBufferLayoutMake(scratch);
    GLVertexBufferLayoutPushAttributeF32(&context.glInfo.vbLayout, 2);
    GLVertexBufferLayoutPushAttributeF32(&context.glInfo.vbLayout, 3);
    GLVertexArrayAddBuffer(
        context.glInfo.va, &context.glInfo.vb, &context.glInfo.vbLayout);

    GL_CALL(glUseProgram(shader));

    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    return context;
}

void
DrawBegin(DrawContext *context)
{
    GL_CALL(glUseProgram(context->shader));
    UNUSED(context);
}

void
DrawClear(DrawContext *context, f32 r, f32 g, f32 b)
{
    UNUSED(context);
    GLClearEx(r, g, b, 1, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
DrawRectangle(
    DrawContext *context, f32 x, f32 y, f32 width, f32 height, f32 scale,
    f32 rotate, Color4RGBA color)
{
    UNUSED(color);
    UNUSED(rotate);
    UNUSED(scale);

    Vertex vertexes[4] = {};
    Color3RGB rectangleColor = {color.r, color.g, color.b};
    GenerateRectangleVertexes(vertexes, x, y, width, height, rectangleColor);
    static const u32 indicies[] = {0, 1, 2, 0, 2, 3};

    GLVertexBufferSendData(&context->glInfo.vb, vertexes, sizeof(Vertex) * 4);
    GLElementBufferSendData(
        &context->glInfo.eb, indicies, STATIC_ARRAY_LENGTH(indicies));

    mat4 model = {0};
    glm_mat4_copy(GLM_MAT4_IDENTITY, model);

    // vec3 position = { x, y, 0 };
    // glm_translate(model, position);

    // vec3 size = { width, height, 0 };
    // glm_scale(model, size);

    mat4 projection = {0};
    Camera_GetProjectionMatix(context->camera, &projection);

    GLShaderSetUniformM4F32(
        context->shader, context->glInfo.uniformLocationModel, (f32 *)model);
    GLShaderSetUniformM4F32(
        context->shader, context->glInfo.uniformLocationProjection,
        (f32 *)projection);

    GLDrawElements(
        &context->glInfo.eb, &context->glInfo.vb, context->glInfo.va);
}

void
DrawEnd(DrawContext *context)
{
    UNUSED(context);
}
