/*
 * FILE      demos\breakout\render.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "render.h"

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
            glm_rad(camera->fov), (f32)windowRect.width / (f32)windowRect.height,
            //                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
            // NOTE(gr3yknigh1): Viewport width and height (not the window).
            // [2024/09/22]
            camera->near, camera->far, *projection);
    } else if (camera->viewMode == CAMERA_VIEW_MODE_ORTHOGONAL) {
        glm_ortho(
            0, (f32)windowRect.width, 0, (f32)windowRect.height, camera->near, camera->far, *projection);
    } else {
        ASSERT_ISTRUE(false);
    }
}
