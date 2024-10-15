/*
 * FILE      gfs_render_opengl.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs/render_opengl.h"

#include "gfs/assert.h"
#include "gfs/platform.h"
#include "gfs/memory.h"
#include "gfs/string.h"
#include "gfs/types.h"

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
GLVertexArrayAddBuffer(GLVertexArray va, const GLVertexBuffer *vb, const GLVertexBufferLayout *layout) {

    GL_CALL(glBindVertexArray(va));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vb->id));

    u32 offset = 0;

    for (u32 attributeIndex = 0; attributeIndex < layout->attributesCount; ++attributeIndex) {
        GLAttribute *attribute = layout->attributes + attributeIndex;

        GL_CALL(glEnableVertexAttribArray(attributeIndex));
        GL_CALL(glVertexAttribPointer(
            attributeIndex, attribute->count, attribute->type, attribute->isNormalized, layout->stride,
            (void *)offset));

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
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, dataBufferSize, dataBuffer, GL_DYNAMIC_DRAW /* GL_STATIC_DRAW */));

    return buffer;
}


void
GLVertexBufferSendData(GLVertexBuffer *buffer, const void *dataBuffer, usize dataBufferSize) {
    buffer->data = dataBuffer;
    buffer->size = dataBufferSize;

    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, buffer->id));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, buffer->size, buffer->data, GL_DYNAMIC_DRAW /* GL_STATIC_DRAW */));
}


GLIndexBuffer
GLIndexBufferMake(const void *indexBuffer, usize indexBufferSize) {
    GLuint ebo = 0;

    GL_CALL(glGenBuffers(1, &ebo));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, indexBuffer, GL_DYNAMIC_DRAW /* GL_STATIC_DRAW */));

    return (GLIndexBuffer)ebo;
}

GLElementBuffer
GLElementBufferMake(const u32 *elements, u64 count) {
    GLElementBuffer eb;

    usize elementsBufferSize = sizeof(elements[0]) * count;

    GL_CALL(glGenBuffers(1, &(eb.id)));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eb.id));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementsBufferSize, (const void *)elements, GL_DYNAMIC_DRAW /* GL_STATIC_DRAW */));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eb.id)); // @cleanup

    eb.elements = elements;
    eb.count = count;

    return eb;
}

void
GLElementBufferSendData(GLElementBuffer *buffer, const u32 *indicies, u64 count) {
    buffer->elements = indicies;
    buffer->count = count;

    usize elementsBufferSize = sizeof(buffer->elements[0]) * buffer->count;

    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->id));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementsBufferSize, (const void *)buffer->elements, GL_DYNAMIC_DRAW /* GL_STATIC_DRAW */));

}


GLVertexBufferLayout
GLVertexBufferLayoutMake(Scratch *scratch) {
    GLVertexBufferLayout layout = {0};

    layout.scratch = scratch;
    layout.attributes = ScratchAllocZero(scratch,
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
GLDrawElements(const GLElementBuffer *eb, const GLVertexBuffer *vb, GLVertexArray va) {
    UNUSED(vb);

    GL_CALL(glBindVertexArray(va));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eb->id));
    GL_CALL(glDrawElements(GL_TRIANGLES, eb->count, GL_UNSIGNED_INT, 0));
}

void
GLDrawTriangles(const GLVertexBuffer *vb, const GLVertexBufferLayout *layout, GLVertexArray va) {
    GL_CALL(glBindVertexArray(va));
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, vb->size / layout->stride));
}

void
GLDrawMesh(const Mesh *mesh) {
    GLDrawElements(&mesh->elementBuffer, &mesh->vertexBuffer, mesh->vertexArray);
}

GLShaderID
GLCompileShaderFromFile(Scratch *scratch, cstring8 sourceFilePath, GLShaderType shaderType) {
    ASSERT_ISTRUE(IsPathExists(sourceFilePath));
    ASSERT_NOTEQ(shaderType, GL_SHADER_TYPE_NONE);

    FileOpenResult sourceFileOpenResult = FileOpenEx(sourceFilePath, scratch, PERMISSION_READ);
    ASSERT_ISOK(sourceFileOpenResult.code);

    FileHandle *sourceHandle = sourceFileOpenResult.handle;
    ASSERT_ISTRUE(FileHandleIsValid(sourceHandle));

    usize sourceFileSize = FileGetSize(sourceHandle);
    ASSERT_NONZERO(sourceFileSize);

    Scratch tempScratch = TempScratchMake(scratch, sourceFileSize);

    void *sourceBuffer = ScratchAllocZero(&tempScratch, sourceFileSize);
    ASSERT_NONNULL(sourceBuffer);

    FileLoadResultCode sourceLoadResult = FileLoadToBuffer(sourceHandle, sourceBuffer, sourceFileSize, NULL);
    ASSERT_ISOK(sourceLoadResult);

    ASSERT_ISOK(FileClose(sourceHandle));

    GLShaderID shaderID = GLCompileShader(scratch, sourceBuffer, shaderType);

    TempScratchClean(&tempScratch, scratch);

    return shaderID;
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
GLCompileShader(Scratch *scratch, cstring8 shaderSource, GLShaderType shaderType) {
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
        GL_CALL(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &compilationLogLength));

        if (compilationLogLength > 0) {

            char8 *compilationInfoLog = ScratchAlloc(scratch, compilationLogLength); // @cleanup XXX
            GL_CALL(glGetShaderInfoLog(shaderId, GL_INFO_LOG_LENGTH, NULL, compilationInfoLog));
            PutString(compilationInfoLog);

        }

        ThrowDebugBreak(); // @cleanup Hack
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
            char8 *linkInfoLog = ScratchAlloc(scratch, linkLogLength);  // @cleanup XXX
            GL_CALL(glGetProgramInfoLog(programID, GL_INFO_LOG_LENGTH, NULL, linkInfoLog));

            PutString(linkInfoLog);
        }

        ThrowDebugBreak(); // @cleanup Hack
        return 0;
    }

    return programID;
}

static inline GLenum
OpenGL_ConvertColorLayoutToOpenGLValues(ColorLayout layout) {
    if (layout == COLOR_LAYOUT_BGR) {
        return GL_BGR;
    }

    if (layout == COLOR_LAYOUT_RGBA) {
        return GL_RGBA;
    }

    if (layout == COLOR_LAYOUT_BGRA) {
        return GL_BGRA;
    }

    if (layout == COLOR_LAYOUT_RGB) {
        return GL_RGB;
    }

    return (GLenum)-1;
}

GLTexture
GLTextureMakeFromBMPicture(const BMPicture *picture, ColorLayout colorLayout) {
    GLuint texture;
    GL_CALL(glGenTextures(1, &texture));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, texture));
    GL_CALL(glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, picture->dibHeader.width, picture->dibHeader.height, 0,
        OpenGL_ConvertColorLayoutToOpenGLValues(colorLayout), GL_UNSIGNED_BYTE, picture->data));
    GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));

    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    // Unbind (cleanup)
    GL_CALL((glBindTexture(GL_TEXTURE_2D, 0)));

    return (GLTexture)texture;
}

GFS_API GLUniformLocation
GLShaderFindUniformLocation(GLShaderProgramID shader, cstring8 name) {
    u32 uniformLocation = 0;
    GL_CALL_O(glGetUniformLocation(shader, name), &uniformLocation);
    return uniformLocation;
}

void
GLShaderSetUniformF32(GLShaderProgramID shader, GLUniformLocation location, f32 value) {
    UNUSED(shader);
    GL_CALL(glUniform1f(location, value));
}

void
GLShaderSetUniformV3F32(GLShaderProgramID shader, GLUniformLocation location, f32 x, f32 y, f32 z) {
    UNUSED(shader);
    GL_CALL(glUniform3f(location, x, y, z));
}

void
GLShaderSetUniformI32(GLShaderProgramID shader, GLUniformLocation location, i32 value) {
    UNUSED(shader);
    GL_CALL(glUniform1i(location, value));
}

void
GLShaderSetUniformM4F32(GLShaderProgramID shader, GLUniformLocation location, f32 *items) {
    UNUSED(shader);
    GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, items));
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
            printBuffer, "E: [GL] Error code=%s[%i] - '%s' at %s:%lu\n", GLGetErrorString(errorCode), errorCode,
            expression, sourceFile, sourceLine);
        THROW(printBuffer);
    }
}

Mesh *
GLGetCubeMesh(Scratch *scratch, GLVertexesOrientation orientation) {
    ASSERT_EQ(orientation, GL_COUNTER_CLOCK_WISE); // TODO: Implement clockwise mesh.

    Mesh *mesh = ScratchAllocZero(scratch, sizeof(Mesh));  // XXX

    // NOTE(gr3yknigh1) Counter Clock-wise. [2024/10/03]
    // NOTE(gr3yknigh1): Our UVs messed up. So leave that for later, when texture atlas will be implemented.
    // [2024/10/05]
    // clang-format off
    static const f32 vertexes[] = {
        // Position    Colors        UV
        // Front face
        0, 1, 1,       1, 1, 1,      0, 1, // [00] top-left
        1, 1, 1,       1, 1, 1,      1, 1, // [01] top-right
        1, 0, 1,       1, 1, 1,      1, 0, // [02] bottom-right
        0, 0, 1,       1, 1, 1,      0, 0, // [03] bottom-left

        // Back face
        0, 1, 0,       1, 1, 1,      1, 1, // [04] top-left
        1, 1, 0,       1, 1, 1,      0, 1, // [05] top-right
        1, 0, 0,       1, 1, 1,      0, 0, // [06] bottom-right
        0, 0, 0,       1, 1, 1,      1, 0, // [07] bottom-left

        // Top face
        0, 1, 0,       1, 1, 1,      0, 1, // [08] top-left
        1, 1, 0,       1, 1, 1,      1, 1, // [09] top-right
        1, 1, 1,       1, 1, 1,      1, 0, // [10] bottom-right
        0, 1, 1,       1, 1, 1,      0, 0, // [11] bottom-left

        // Bottom face
        0, 0, 1,       1, 1, 1,      0, 1, // [12] top-left
        1, 0, 1,       1, 1, 1,      1, 1, // [13] top-right
        1, 0, 0,       1, 1, 1,      1, 0, // [14] bottom-right
        0, 0, 0,       1, 1, 1,      0, 0, // [15] bottom-left

        // Left face
        0, 1, 0,       1, 1, 1,      0, 1, // [16] top-left
        0, 1, 1,       1, 1, 1,      1, 1, // [17] top-right
        0, 0, 0,       1, 1, 1,      0, 0, // [18] bottom-right
        0, 0, 1,       1, 1, 1,      1, 0, // [19] bottom-left

        // Right face
        1, 1, 0,       1, 1, 1,      1, 1, // [20] top-right
        1, 1, 1,       1, 1, 1,      0, 1, // [21] top-left
        1, 0, 0,       1, 1, 1,      1, 0, // [22] bottom-right
        1, 0, 1,       1, 1, 1,      0, 0, // [23] bottom-left
    };
    // clang-format on

    mesh->vertexArray = GLVertexArrayMake();
    mesh->vertexBuffer = GLVertexBufferMake(vertexes, sizeof(vertexes));

    mesh->vertexLayout = GLVertexBufferLayoutMake(scratch);  // XXX
    GLVertexBufferLayoutPushAttributeF32(&mesh->vertexLayout, 3);
    GLVertexBufferLayoutPushAttributeF32(&mesh->vertexLayout, 3);
    GLVertexBufferLayoutPushAttributeF32(&mesh->vertexLayout, 2);

    GLVertexArrayAddBuffer(mesh->vertexArray, &mesh->vertexBuffer, &mesh->vertexLayout);

    // clang-format off
    static const u32 indexes[] = {
        // Front face
        0, 1, 3, // top-left
        3, 1, 2, // bottom-right
        // Back face
        5, 4, 6, // top-left
        6, 4, 7, // bottom-right
        // Top face
        11, 8, 9,  // top-left
        11, 9, 10, // bottom-right
        // Bottom face
        13, 15, 12, // top-left
        15, 13, 14, // bottom-right
        // Left face
        18, 16, 17, // top-left
        18, 17, 19, // bottom-right
        // Right face
        23, 21, 20, // top-left
        23, 20, 22, // bottom-right
    };
    // clang-format on

    mesh->elementBuffer = GLElementBufferMake(indexes, STATIC_ARRAY_LENGTH(indexes));

    return mesh;
}

Mesh *
GLMeshMakeEx(
    Scratch *scratch, const f32 *vertexBuffer, usize vertexBufferSize, const u32 *indexBuffer, u64 indexesCount) {

    // TODO: Do we need to allocate mesh on the heap?
    Mesh *mesh = ScratchAllocZero(scratch, sizeof(Mesh));

    mesh->vertexArray = GLVertexArrayMake();
    mesh->vertexBuffer = GLVertexBufferMake(vertexBuffer, vertexBufferSize);

    // TODO: Customize layout?
    mesh->vertexLayout = GLVertexBufferLayoutMake(scratch);  // XXX
    GLVertexBufferLayoutPushAttributeF32(&mesh->vertexLayout, 3);
    GLVertexBufferLayoutPushAttributeF32(&mesh->vertexLayout, 2);
    GLVertexBufferLayoutPushAttributeF32(&mesh->vertexLayout, 3);

    GLVertexArrayAddBuffer(mesh->vertexArray, &mesh->vertexBuffer, &mesh->vertexLayout);

    mesh->elementBuffer = GLElementBufferMake(indexBuffer, indexesCount);

    return mesh;
}
