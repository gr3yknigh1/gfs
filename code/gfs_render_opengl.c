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
#include "gfs_types.h"

#include <glad/glad.h>

#include <Windows.h> // wsprintf

GLShaderID
GLCompileShaderFromFile(
    Scratch *scratch, cstring8 sourceFilePath, GLShaderType shaderType) {
    ASSERT_ISTRUE(IsPathExists(sourceFilePath));
    ASSERT_NOTEQ(shaderType, GL_SHADER_TYPE_NONE);

    FileOpenResult sourceFileOpenResult =
        FileOpenEx(sourceFilePath, scratch, PLATFORM_PERMISSION_READ);
    ASSERT_ISOK(sourceFileOpenResult.code);

    FileHandle *sourceHandle = sourceFileOpenResult.handle;
    ASSERT_ISTRUE(FileHandleIsValid(sourceHandle));

    usize sourceFileSize = FileGetSize(sourceHandle);
    ASSERT_NONZERO(sourceFileSize);

    void *sourceBuffer = ScratchAllocZero(scratch, sourceFileSize);
    ASSERT_NONNULL(sourceBuffer);

    FileLoadResultCode sourceLoadResult =
        FileLoadToBuffer(sourceHandle, sourceBuffer, sourceFileSize, NULL);
    ASSERT_ISOK(sourceLoadResult);

    // TODO(gr3yknigh1): Close file.

    return GLCompileShader(scratch, sourceBuffer, shaderType);
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
GLCompileShader(
    Scratch *scratch, cstring8 shaderSource, GLShaderType shaderType) {
    ASSERT_NONNULL(shaderSource);
    ASSERT_ISFALSE(CString8IsEmpty(shaderSource));
    ASSERT_NOTEQ(shaderType, GL_SHADER_TYPE_NONE);

    GLShaderID shaderId = 0;

    GLenum glShaderType = OpenGL_ConvertShaderTypeToGLEnum(shaderType);

    GL_CALL_O(glCreateShader(glShaderType), &shaderId);
    GL_CALL(glShaderSource(shaderId, 1, &shaderSource, NULL));
    GL_CALL(glCompileShader(shaderId));

    GLint compilationStatus = GL_TRUE;
    GL_CALL(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compilationStatus));

    if (compilationStatus == GL_FALSE) {
        GLint compilationLogLength = 0;
        GL_CALL(
            glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &compilationLogLength));

        if (compilationLogLength > 0) {
            char8 *compilationInfoLog =
                ScratchAlloc(scratch, compilationLogLength);
            GL_CALL(glGetShaderInfoLog(
                shaderId, GL_INFO_LOG_LENGTH, NULL, compilationInfoLog));

            PutString(compilationInfoLog);
        }

        ThrowDebugBreak(); // XXX
        return 0;
    }

    return shaderId;
}

GLShaderProgramID
GLLinkShaderProgram(Scratch *scratch, const GLShaderProgramLinkData *data) {
    ASSERT_NONNULL(data);
    ASSERT_NONZERO(data->vertexShader);
    ASSERT_NONZERO(data->fragmentShader);

    GLShaderProgramID programID = 0;
    GL_CALL_O(glCreateProgram(), &programID);

    GL_CALL(glAttachShader(programID, data->vertexShader));
    GL_CALL(glAttachShader(programID, data->fragmentShader));
    GL_CALL(glLinkProgram(programID));

    GL_CALL(glValidateProgram(programID));

    GLint linkStatus = GL_TRUE;
    GL_CALL(glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus));

    if (linkStatus == GL_FALSE) {
        GLint linkLogLength;
        GL_CALL(glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &linkLogLength));

        if (linkLogLength > 0) {
            char8 *linkInfoLog = ScratchAlloc(scratch, linkLogLength);
            GL_CALL(glGetProgramInfoLog(
                programID, GL_INFO_LOG_LENGTH, NULL, linkInfoLog));

            PutString(linkInfoLog);
        }

        ThrowDebugBreak(); // XXX
        return 0;
    }

    return programID;
}

void
GLShaderSetUniformF32(GLShaderProgramID shader, cstring8 name, f32 value) {
    GL_CALL(glUseProgram(shader));

    // TODO(gr3yknigh1: Error handle);
    u32 uniformLocation = 0;
    GL_CALL_O(glGetUniformLocation(shader, name), &uniformLocation);
    GL_CALL(glUniform1f(uniformLocation, value));
}

void
GLShaderSetUniformV3F32(
    GLShaderProgramID shader, cstring8 name, f32 x, f32 y, f32 z) {
    GL_CALL(glUseProgram(shader));

    // TODO(gr3yknigh1: Error handle);
    u32 uniformLocation = 0;
    GL_CALL_O(glGetUniformLocation(shader, name), &uniformLocation);
    GL_CALL(glUniform3f(uniformLocation, x, y, z));
}

void
GLShaderSetUniformI32(GLShaderProgramID shader, cstring8 name, i32 value) {
    GL_CALL(glUseProgram(shader));

    // TODO(gr3yknigh1: Error handle);
    u32 uniformLocation = 0;
    GL_CALL_O(glGetUniformLocation(shader, name), &uniformLocation);
    GL_CALL(glUniform1i(uniformLocation, value));
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
