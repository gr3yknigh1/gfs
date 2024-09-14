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

typedef u32 GLShaderID;
typedef u32 GLShaderProgramID;

typedef enum {
    GL_SHADER_TYPE_NONE,
    GL_SHADER_TYPE_FRAG,
    GL_SHADER_TYPE_VERT,
    GL_SHADER_TYPE_COUNT,
} GLShaderType;

/*
 * @breaf Not implemented!
 * */
GLShaderType GLShaderFileTypeDetect(cstring8 shaderFilePath);

GLShaderID GLCompileShaderFromFile(ScratchAllocator *allocator, cstring8 shaderFilePath, GLShaderType shaderType);
GLShaderID GLCompileShader(cstring8 shaderSourceString, GLShaderType shaderType);

typedef struct {
    GLShaderID vertexShader;
    GLShaderID fragShader;
} GLShaderProgramLinkData;

GLShaderProgramID GLLinkShaderProgram(const GLShaderProgramLinkData *data);

cstring8 GLGetErrorString(i32 errorCode);

void GLClearErrors(void);
void GLAssertNoErrors(cstring8 expression, cstring8 sourceFile, u64 sourceLine);

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

#endif // GFS_RENDER_OPENGL_H_INCLUDED
