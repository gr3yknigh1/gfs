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

typedef struct Glyph {
    GLTexture texture;
    Vector2F32 size;
    Vector2F32 bearing;
    u32 xAdvance;
} Glyph;

typedef struct Font {
    Glyph *glyphs;
    usize glyphsCount;

    u32 pixelWidth;
    u32 pixelHeight;
} Font;

Font *Font_Make(cstring8 fontPath, u32 pixelWidth, u32 pixelHeight);
void Font_Destroy(Font *font);

/*
 * @breaf 2D Draw context.
 */
typedef struct DrawContext {
    Camera *camera;
    GLShaderProgramID defaultShader;

    mat4 model;
    mat4 projection;

    struct {
        GLVertexArray va;
        GLVertexBuffer vb;
        GLElementBuffer eb;
        GLShaderProgramID shader;
        GLVertexBufferLayout layout;

        GLUniformLocation uniformLocationModel;
        GLUniformLocation uniformLocationColor;
        GLUniformLocation uniformLocationProjection;
    } rectDrawInfo;

    struct {
        GLVertexArray va;
        GLVertexBuffer vb;
        GLShaderProgramID shader;
        GLVertexBufferLayout layout;

        Font *selectedFont;

        GLUniformLocation uniformLocationTexture;
        GLUniformLocation uniformLocationColor;
        GLUniformLocation uniformLocationProjection;
    } textDrawInfo;
} DrawContext;

DrawContext DrawContext_MakeEx(
    Scratch *scratch, Camera *camera, GLShaderProgramID rectangleShader,
    GLShaderProgramID textShader);

void DrawContext_SelectFont(DrawContext *context, Font *font);

void DrawBegin(DrawContext *context);

void DrawClear(DrawContext *context, f32 r, f32 g, f32 b);

void DrawRectangle(
    DrawContext *context, f32 x, f32 y, f32 width, f32 height, f32 scale,
    f32 rotate, Color4RGBA color);

void DrawString(
    DrawContext *context, f32 x, f32 y, cstring8 s, f32 scale,
    Color4RGBA color);

void DrawEnd(DrawContext *context);

#endif // BREAKOUT_RENDER_H_INCLUDED
