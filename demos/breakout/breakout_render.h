#if !defined(BREAKOUT_RENDER_H_INCLUDED)
/*
 * FILE      demos\breakout\breakout_render.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define BREAKOUT_RENDER_H_INCLUDED

#include <cglm/cglm.h>

#include <gfs/platform.h>
#include <gfs/render.h>
#include <gfs/types.h>
#include <gfs/render_opengl.h>

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

void Camera_GetProjectionMatix(Camera *camera, mat4 *projection);

typedef struct {
    GLTexture texture;
} SpriteGLInfo;

typedef struct {
    void *data;
    i32 width;
    i32 height;

    SpriteGLInfo glInfo;
} Sprite;

typedef enum {
    FILE_TYPE_NONE,
    FILE_TYPE_BMPICTURE
} FileType;

void Sprite_LoadFromFile(Sprite *out, cstring8 filePath, FileType type);

typedef struct {
    GLVertexArray va;
    GLVertexBuffer vb;
    GLElementBuffer eb;
    GLVertexBufferLayout vbLayout;

    GLUniformLocation uniformLocationModel;
    GLUniformLocation uniformLocationProjection;
} DrawContextGLInfo;

typedef struct {
    Camera *camera;
    GLShaderProgramID shader;
    DrawContextGLInfo glInfo;
} DrawContext;

DrawContext DrawContext_MakeEx(Scratch *scratch, Camera *camera, GLShaderProgramID shader);

void DrawBegin(DrawContext *context);

void DrawClear(DrawContext *context, f32 r, f32 g, f32 b);

void DrawRectangle(DrawContext *context, f32 x, f32 y, f32 width, f32 height, f32 scale, f32 rotate, Color4RGBA color);
void DrawSprite(DrawContext *context, f32 x, f32 y, Sprite *sprite, f32 scale, f32 rotate);

void DrawEnd(DrawContext *context);


#endif // BREAKOUT_RENDER_H_INCLUDED
