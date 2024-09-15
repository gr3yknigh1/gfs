/*
 * FILE      gfs_render_opengl.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs_render_opengl.h"

#include "gfs_assert.h"
#include "gfs_platform.h"
#include "gfs_memory.h"
#include "gfs_string.h"

#include <glad/glad.h>

#include <Windows.h> // wsprintf

GLShaderID
GLCompileShaderFromFile(
    Scratch *allocator, cstring8 sourceFilePath,
    GLShaderType shaderType) {
    ASSERT_ISTRUE(IsPathExists(sourceFilePath));
    ASSERT_NOTEQ(shaderType, GL_SHADER_TYPE_NONE);

    FileOpenResult sourceFileOpenResult =
        FileOpenEx(sourceFilePath, allocator, PLATFORM_PERMISSION_READ);
    ASSERT_ISOK(sourceFileOpenResult.code);

    FileHandle *sourceHandle = sourceFileOpenResult.handle;
    ASSERT_ISTRUE(FileHandleIsValid(sourceHandle));

    usize sourceFileSize = FileGetSize(sourceHandle);
    ASSERT_NONZERO(sourceFileSize);

    void *sourceBuffer = ScratchAllocZero(allocator, sourceFileSize);
    ASSERT_NONNULL(sourceBuffer);

    FileLoadResultCode sourceLoadResult =
        FileLoadToBuffer(sourceHandle, sourceBuffer, sourceFileSize, NULL);
    ASSERT_ISOK(sourceLoadResult);

    return GLCompileShader(sourceBuffer, shaderType);
}

static GLenum
OpenGL_ConvertShaderTypeToGLEnum(GLShaderType type) {
    if (type == GL_SHADER_TYPE_VERT) {
        return GL_VERTEX_SHADER;
    }

    if (type == GL_SHADER_TYPE_FRAG) {
        return GL_FRAGMENT_SHADER;
    }

    return 0;
}

GLShaderID
GLCompileShader(cstring8 shaderSource, GLShaderType shaderType) {
    ASSERT_NONNULL(shaderSource);
    ASSERT_ISFALSE(CString8IsEmpty(shaderSource));
    ASSERT_NOTEQ(shaderType, GL_SHADER_TYPE_NONE);

    GLShaderID shaderId = 0;

    GLenum glShaderType = OpenGL_ConvertShaderTypeToGLEnum(shaderType);

    GL_CALL_O(glCreateShader(glShaderType), &shaderId);
    GL_CALL(glShaderSource(shaderId, 1, &shaderSource, NULL));
    GL_CALL(glCompileShader(shaderId));

    GLint compileStatus = 0;
    GL_CALL(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus));

    if (compileStatus == GL_FALSE) {

        return 0;
    }

    return shaderId;
}

cstring8
GLGetErrorString(i32 errorCode) {
    switch (errorCode) {
    case GL_NO_ERROR:
        return STRINGIFY(GL_NO_ERROR);
    case GL_INVALID_ENUM:
        return STRINGIFY(GL_INVALID_ENUM);
    case GL_INVALID_VALUE:
        return STRINGIFY(GL_INVALID_VALUE);
    case GL_INVALID_OPERATION:
        return STRINGIFY(GL_INVALID_OPERATION);
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return STRINGIFY(GL_INVALID_FRAMEBUFFER_OPERATION);
    case GL_OUT_OF_MEMORY:
        return STRINGIFY(GL_OUT_OF_MEMORY);
    case GL_STACK_UNDERFLOW:
        return STRINGIFY(GL_STACK_UNDERFLOW);
    case GL_STACK_OVERFLOW:
        return STRINGIFY(GL_STACK_OVERFLOW);
    }

    return "GL_UNKNOWN_ERROR";
}

void
GLClearErrors(void) {
    while (glGetError() != GL_NO_ERROR) {
    }
}

void
GLAssertNoErrors(cstring8 expression, cstring8 sourceFile, u64 sourceLine) {
    GLenum errorCode = GL_NO_ERROR;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        char8 printBuffer[KILOBYTES(1)];
        wsprintf(
            printBuffer, "E: [GL] Error code=%s[%i] - '%s' at %s:%lu\n",
            GLGetErrorString(errorCode), errorCode, expression, sourceFile,
            sourceLine);
        THROW(printBuffer);
    }
}
