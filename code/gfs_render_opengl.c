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

GLVertexArray
GLVertexArrayMake(void) {
    GLuint vao = 0;
    GL_CALL(glGenVertexArrays(1, &vao));
    GL_CALL(glBindVertexArray(vao));
    return (GLVertexArray)vao;
}

void
GLVertexArrayAddBuffer(
    GLVertexArray va, const GLVertexBuffer *vb,
    const GLVertexBufferLayout *layout) {

    GL_CALL(glBindVertexArray(va));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vb->id));

    u64 offset = 0;

    for (u64 attributeIndex = 0; attributeIndex < layout->attributesCount;
         ++attributeIndex) {
        GLAttribute *attribute = layout->attributes + attributeIndex;

        GL_CALL(glEnableVertexAttribArray(attributeIndex));
        GL_CALL(glVertexAttribPointer(
            attributeIndex, attribute->count, attribute->type,
            attribute->isNormalized, layout->stride, (void *)offset));

        offset += attribute->size * attribute->count;
    }
}

GLVertexBuffer
GLVertexBufferMake(const void *dataBuffer, usize dataBufferSize) {
    GLVertexBuffer buffer = {0};

    buffer.id = 0;
    buffer.data = dataBuffer;
    buffer.size = dataBufferSize;

    GL_CALL(glGenBuffers(1, &buffer.id));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, buffer.id));
    GL_CALL(glBufferData(
        GL_ARRAY_BUFFER, dataBufferSize, dataBuffer, GL_STATIC_DRAW));

    return buffer;
}

GLIndexBuffer
GLIndexBufferMake(const void *indexBuffer, usize indexBufferSize) {
    GLuint ebo = 0;

    GL_CALL(glGenBuffers(1, &ebo));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
    GL_CALL(glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, indexBuffer, GL_STATIC_DRAW));

    return (GLIndexBuffer)ebo;
}

GLVertexBufferLayout
GLVertexBufferLayoutMake(Scratch *scratch) {
    GLVertexBufferLayout layout = {0};

    layout.scratch = scratch;
    layout.attributes = ScratchAllocZero(
        scratch,
        KILOBYTES(1)); // TODO(gr3yknigh1): replace with generic allocator
    layout.attributesCount = 0;
    layout.stride = 0;

    return layout;
}

void
GLVertexBufferLayoutPushAttributeF32(GLVertexBufferLayout *layout, u32 count) {
    usize attributeSize = sizeof(f32);

    GLAttribute *attribute = layout->attributes + layout->attributesCount;
    attribute->isNormalized = false;
    attribute->type = GL_FLOAT;
    attribute->count = count;
    attribute->size = attributeSize;

    layout->attributesCount += 1;
    layout->stride += attributeSize * count;
}

void
GLClear(f32 r, f32 g, f32 b, f32 a) {
    GL_CALL(glClearColor(r, g, b, a));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void
GLClearEx(f32 r, f32 g, f32 b, f32 a, i32 clearMask) {
    GL_CALL(glClearColor(r, g, b, a));
    GL_CALL(glClear(clearMask));
}

void
GLDrawTriangles(
    const GLVertexBuffer *vb, const GLVertexBufferLayout *layout,
    GLVertexArray va, GLShaderProgramID shader, GLTexture texture) {

    GL_CALL(glUseProgram(shader));

    GL_CALL(
        glActiveTexture(GL_TEXTURE0)); // TODO(gr3yknigh1): Investigate in multi
                                       // texture support. [2024/09/22]
    GL_CALL(glBindTexture(GL_TEXTURE_2D, texture));

    GL_CALL(glBindTexture(GL_TEXTURE_2D, texture));
    GL_CALL(glBindVertexArray(va));

    // GL_CALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
    // GL_CALL(
    //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib)); // Renders without
    //     this
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, vb->size / layout->stride));

    // TODO(gr3yknigh1): Fix this
    // GL_CALL(glDrawElements(
    //     GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]),
    //     GL_UNSIGNED_INT, 0));
}

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

    ASSERT_ISOK(FileClose(sourceHandle));

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

GLTexture
GLTextureMakeFromBMPicture(const BMPicture *picture) {

    GLuint texture;
    GL_CALL(glGenTextures(1, &texture));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, texture));
    GL_CALL(glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, picture->dibHeader.width,
        picture->dibHeader.height, 0, GL_BGR, GL_UNSIGNED_BYTE, picture->data));
    GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

    GL_CALL(
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL_CALL(
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));

    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    return (GLTexture)texture;
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

void
GLShaderSetUniformM4F32(GLShaderProgramID shader, cstring8 name, f32 *items) {
    GL_CALL(glUseProgram(shader));

    // TODO(gr3yknigh1: Error handle);
    u32 uniformLocation = 0;
    GL_CALL_O(glGetUniformLocation(shader, name), &uniformLocation);
    GL_CALL(glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, items));
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

// TODO(gr3yknigh1): Implement printing debug information [2024/09/22]
#if 0
    const byte *glVendor = glGetString(GL_VENDOR);
    const byte *glRenderer = glGetString(GL_RENDERER);
    const byte *glVersion = glGetString(GL_VERSION);
    const byte *glExtensions = glGetString(GL_EXTENSIONS);
    const byte *glShaderLanguage = glGetString(GL_SHADING_LANGUAGE_VERSION);

    UNUSED(glVendor);
    UNUSED(glRenderer);
    UNUSED(glVersion);
    UNUSED(glExtensions);
    UNUSED(glShaderLanguage);
#endif
