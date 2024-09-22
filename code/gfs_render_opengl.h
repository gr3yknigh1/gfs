#if !defined(GFS_RENDER_OPENGL_H_INCLUDED)
/*
 * FILE      gfs_render_opengl.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_RENDER_OPENGL_H_INCLUDED

#include "gfs_types.h"
#include "gfs_macros.h"
#include "gfs_memory.h"
#include "gfs_bmp.h"

typedef u32 GLShaderID;
typedef u32 GLShaderProgramID;

typedef u32 GLTexture;

typedef u32 GLVertexArray;

typedef struct {
    u32 id;
    const void *data;
    usize size;
} GLVertexBuffer;

typedef u32 GLIndexBuffer;

typedef struct {
    bool isNormalized; // I don't like this bool
    u32 type;
    u32 count;
    usize size;
} GLAttribute;

typedef struct {
    Scratch *scratch;

    GLAttribute *attributes;
    u64 attributesCount;

    u64 stride;
} GLVertexBufferLayout;

GLVertexArray GLVertexArrayMake(void);
void GLVertexArrayAddBuffer(
    GLVertexArray va, const GLVertexBuffer *vb,
    const GLVertexBufferLayout *layout);

GLVertexBuffer GLVertexBufferMake(const void *dataBuffer, usize dataBufferSize);

GLIndexBuffer GLIndexBufferMake(const void *indexBuffer, usize indexBufferSize);

GLVertexBufferLayout GLVertexBufferLayoutMake(Scratch *scratch);

void GLVertexBufferLayoutPushAttributeF32(
    GLVertexBufferLayout *layout, u32 count);

void GLClear(f32 r, f32 g, f32 b, f32 a);
void GLClearEx(f32 r, f32 g, f32 b, f32 a, i32 clearMask);

void GLDrawTriangles(
    const GLVertexBuffer *vb, const GLVertexBufferLayout *layout,
    GLVertexArray va, GLShaderProgramID shader, GLTexture texture);

typedef enum {
    GL_SHADER_TYPE_NONE,
    GL_SHADER_TYPE_FRAG,
    GL_SHADER_TYPE_VERT,
    GL_SHADER_TYPE_COUNT,
} GLShaderType;

/*
 * @breaf Compiles OpenGL shader from source file.
 *
 * @return Shader ID
 * */
GLShaderID GLCompileShaderFromFile(
    Scratch *scratch, cstring8 shaderFilePath, GLShaderType shaderType);

/*
 * @breaf Compiles OpenGL shader.
 *
 * @return Shader ID
 * */
GLShaderID GLCompileShader(
    Scratch *scratch, cstring8 shaderSourceString, GLShaderType shaderType);

typedef struct {
    GLShaderID vertexShader;
    GLShaderID fragmentShader;
} GLShaderProgramLinkData;

GLShaderProgramID GLLinkShaderProgram(
    Scratch *scratch, const GLShaderProgramLinkData *data);

GLTexture GLTextureMakeFromBMPicture(const BMPicture *picture);

void GLShaderSetUniformF32(GLShaderProgramID shader, cstring8 name, f32 value);
void GLShaderSetUniformV3F32(
    GLShaderProgramID shader, cstring8 name, f32 x, f32 y, f32 z);
void GLShaderSetUniformI32(GLShaderProgramID shader, cstring8 name, i32 value);
void GLShaderSetUniformM4F32(
    GLShaderProgramID shader, cstring8 name, f32 *value);

cstring8 GLGetErrorString(i32 errorCode);

void GLClearErrors(void);
void GLAssertNoErrors(cstring8 expression, cstring8 sourceFile, u64 sourceLine);

#define GL_CALL(EXPR)                                                          \
    do {                                                                       \
        GLClearErrors();                                                       \
        (EXPR);                                                                \
        GLAssertNoErrors(STRINGIFY(EXPR), __FILE__, __LINE__);                 \
    } while (0)

#define GL_CALL_O(EXPR, OUT)                                                   \
    do {                                                                       \
        GLClearErrors();                                                       \
        *(OUT) = (EXPR);                                                       \
        GLAssertNoErrors(STRINGIFY(EXPR), __FILE__, __LINE__);                 \
    } while (0)

#endif // GFS_RENDER_OPENGL_H_INCLUDED
