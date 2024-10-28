#if !defined(BREAKOUT_RENDER_H_INCLUDED)
/*
 * FILE      demos\breakout\render.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define BREAKOUT_RENDER_H_INCLUDED

#include <cglm/cglm.h>

#include <gfs/platform.h>

typedef enum {
    CAMERA_VIEW_MODE_PERSPECTIVE,
    CAMERA_VIEW_MODE_ORTHOGONAL,
} CameraViewMode;

typedef struct {
    vec3 position;
    vec3 front;
    vec3 up;

    f32 yaw;
    f32 pitch;

    f32 speed;
    f32 sensitivity;
    f32 fov;

    f32 near;
    f32 far;

    CameraViewMode viewMode;

    Window *window;
} Camera;

Camera Camera_Make(Window *window, f32 near, f32 far, CameraViewMode viewMode);

void Camera_Rotate(Camera *camera, f32 xOffset, f32 yOffset);
void Camera_HandleInput(Camera *camera);

void Camera_GetViewMatix(Camera *camera, mat4 *view);
void Camera_GetProjectionMatix(Camera *camera, mat4 *projection);

#endif // BREAKOUT_RENDER_H_INCLUDED
