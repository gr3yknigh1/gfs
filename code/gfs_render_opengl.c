/*
 * FILE      gfs_render_opengl.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs_render_opengl.h"

#include "gfs_assert.h"
#include "gfs_platform.h"
#include "gfs_memory.h"

#include <glad/glad.h>

#include <Windows.h> // wsprintf

GLShaderID
GLCompileShaderFromFile(ScratchAllocator *allocator, cstring8 shaderFilePath, GLShaderType shaderType) {
    ASSERT_ISTRUE(PlatformIsPathExists(shaderFilePath));
    ASSERT_NOTEQ(shaderType, GL_SHADER_TYPE_NONE);

    PlatformFileOpenResult shaderFileOpenResult =
        PlatformFileOpenEx(shaderFilePath, allocator, PLATFORM_PERMISSION_READ);
    ASSERT_ISOK(shaderFileOpenResult.code);
    ASSERT_ISTRUE(PlatformFileHandleIsValid(shaderFileOpenResult.handle));

    PlatformFileHandle *shaderFileHandle = shaderFileOpenResult.handle;

    // TODO(ilya.a): Replace with checking file size and allocating according to file size. [2024/09/12]
    void *shaderFileBuffer = ScratchAllocatorAlloc(allocator, KILOBYTES(1));
    MemoryZero(shaderFileBuffer, KILOBYTES(1));
    PlatformFileLoadResult fileLoadResult =
        PlatformFileLoadToBuffer(shaderFileHandle, shaderFileBuffer, KILOBYTES(1), NULL);
    ASSERT_ISOK(fileLoadResult);

    return GLCompileShader(shaderFileBuffer, shaderType);
}

GLShaderID
GLCompileShader(cstring8 shaderSourceString, GLShaderType shaderType) {
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
    default:
        return "GL_UNHANDLED_ERROR";
    }
    THROW("Unreachable!");
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
            printBuffer, "E: [GL] Error code=%s[%i] - '%s' at %s:%lu\n", GLGetErrorString(errorCode), errorCode,
            expression, sourceFile, sourceLine);
        THROW(printBuffer);
    }
}
