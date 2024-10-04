#if !defined(GFS_RENDER_OPENGL_H_INCLUDED)
/*
 * FILE      gfs_render_opengl.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_RENDER_OPENGL_H_INCLUDED

#include "gfs/types.h"
#include "gfs/macros.h"
#include "gfs/memory.h"
#include "gfs/bmp.h"

typedef u32 GLShaderID;
typedef u32 GLShaderProgramID;
typedef u32 GLUniformLocation;

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

GFS_API GLVertexArray GLVertexArrayMake(void);
GFS_API void GLVertexArrayAddBuffer(GLVertexArray va, const GLVertexBuffer *vb, const GLVertexBufferLayout *layout);

GFS_API GLVertexBuffer GLVertexBufferMake(const void *dataBuffer, usize dataBufferSize);

GFS_API GLIndexBuffer GLIndexBufferMake(const void *indexBuffer, usize indexBufferSize);

GFS_API GLVertexBufferLayout GLVertexBufferLayoutMake(Scratch *scratch);

GFS_API void GLVertexBufferLayoutPushAttributeF32(GLVertexBufferLayout *layout, u32 count);

GFS_API void GLClear(f32 r, f32 g, f32 b, f32 a);
GFS_API void GLClearEx(f32 r, f32 g, f32 b, f32 a, i32 clearMask);

GFS_API void GLDrawTriangles(const GLVertexBuffer *vb, const GLVertexBufferLayout *layout, GLVertexArray va);

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
GFS_API GLShaderID GLCompileShaderFromFile(Scratch *scratch, cstring8 shaderFilePath, GLShaderType shaderType);

/*
 * @breaf Compiles OpenGL shader.
 *
 * @return Shader ID
 * */
GFS_API GLShaderID GLCompileShader(Scratch *scratch, cstring8 shaderSourceString, GLShaderType shaderType);

typedef struct {
    GLShaderID vertexShader;
    GLShaderID fragmentShader;
} GLShaderProgramLinkData;

GFS_API GLShaderProgramID GLLinkShaderProgram(Scratch *scratch, const GLShaderProgramLinkData *data);

GFS_API GLTexture GLTextureMakeFromBMPicture(const BMPicture *picture);

/*
 * @breaf Returns location of uniform.
 *
 * @param shader Shader program id which should be enabled with glUseProgram call.
 * @param name Name of uniform.
 * */
GFS_API GLUniformLocation GLShaderFindUniformLocation(GLShaderProgramID shader, cstring8 name);

GFS_API void GLShaderSetUniformF32(GLShaderProgramID shader, GLUniformLocation location, f32 value);
GFS_API void GLShaderSetUniformV3F32(GLShaderProgramID shader, GLUniformLocation location, f32 x, f32 y, f32 z);
GFS_API void GLShaderSetUniformI32(GLShaderProgramID shader, GLUniformLocation location, i32 value);
GFS_API void GLShaderSetUniformM4F32(GLShaderProgramID shader, GLUniformLocation location, f32 *value);

GFS_API cstring8 GLGetErrorString(i32 errorCode);

GFS_API void GLClearErrors(void);
GFS_API void GLAssertNoErrors(cstring8 expression, cstring8 sourceFile, u64 sourceLine);

#if defined(GFS_OPENGL_DEBUG)

#define GL_CALL(EXPR)                                                                                                  \
    do {                                                                                                               \
        GLClearErrors();                                                                                               \
        (EXPR);                                                                                                        \
        GLAssertNoErrors(STRINGIFY(EXPR), __FILE__, __LINE__);                                                         \
    } while (0)

#define GL_CALL_O(EXPR, OUT)                                                                                           \
    do {                                                                                                               \
        GLClearErrors();                                                                                               \
        *(OUT) = (EXPR);                                                                                               \
        GLAssertNoErrors(STRINGIFY(EXPR), __FILE__, __LINE__);                                                         \
    } while (0)

#else

#define GL_CALL(X) (X)
#define GL_CALL_O(X, O) (*(O) = (X))

#endif

#endif // GFS_RENDER_OPENGL_H_INCLUDED
