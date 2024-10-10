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

typedef union {
    struct {
        f32 s;
        f32 t;
    };
    f32 data[2];
} TexCoords;

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
    u32 id;
    const u32 *elements;
    u64 count;
} GLElementBuffer;

typedef struct {
    Scratch *scratch;

    GLAttribute *attributes;
    u64 attributesCount;

    u64 stride;
} GLVertexBufferLayout;

typedef enum {
    GL_TEXTURE_COLOR_ORDER_BGR,
    GL_TEXTURE_COLOR_ORDER_BGRA,
    GL_TEXTURE_COLOR_ORDER_RGB,
    GL_TEXTURE_COLOR_ORDER_RGBA,
} GLTextureColorOrder;

typedef struct {
    GLVertexArray vertexArray;
    GLVertexBuffer vertexBuffer;
    GLElementBuffer elementBuffer;
    GLVertexBufferLayout vertexLayout;
} Mesh;

typedef enum { GL_CLOCK_WISE, GL_COUNTER_CLOCK_WISE } GLVertexesOrientation;

/* TODO: Customize layout? */
GFS_API Mesh *GLMeshMakeEx(
    Scratch *scratch, const f32 *vertexBuffer, usize vertexBufferSize, const u32 *indexBuffer, u64 indexesCount);

/*
 * @breaf Returns identity cube mesh. Used only for testing perpuses.
 *
 * Example:
 *     ```c
 *          const Mesh *cubeMesh = GLGetCubeMesh(&runtimeScratch, GL_COUNTER_CLOCK_WISE);
 *          // ...
 *          while (true) {
 *              // ...
 *              glm::mat4 model = glm::identity<glm::mat4>();
 *              GLShaderSetUniformM4F32(shader, uniformModelLocation, glm::value_ptr(model));
 *              GLDrawMesh(cubeMesh);
 *          }
 *     ```
 */
GFS_API Mesh *GLGetCubeMesh(Scratch *scratch, GLVertexesOrientation orientation);

GFS_API GLVertexArray GLVertexArrayMake(void);
GFS_API void GLVertexArrayAddBuffer(GLVertexArray va, const GLVertexBuffer *vb, const GLVertexBufferLayout *layout);

GFS_API GLVertexBuffer GLVertexBufferMake(const void *dataBuffer, usize dataBufferSize);

/*
 * @breaf Naive wrapper around element buffer.
 * @deprecated Use GLElementBuffer instead.
 * @cleanup
 */
GFS_API GLIndexBuffer GLIndexBufferMake(const void *indexBuffer, usize indexBufferSize);

/*
 * @breaf Constructs element buffer, which doesn't owns indicies, which was passed in
 *
 * TODO(gr3yknigh1): Copy indicies inside [2024/10/05]
 * TODO(gr3yknigh1): Add option for dynamic draw [2024/10/05]
 */
GFS_API GLElementBuffer GLElementBufferMake(const u32 *indicies, u64 count);

GFS_API GLVertexBufferLayout GLVertexBufferLayoutMake(Scratch *scratch);

GFS_API void GLVertexBufferLayoutPushAttributeF32(GLVertexBufferLayout *layout, u32 count);

GFS_API void GLClear(f32 r, f32 g, f32 b, f32 a);
GFS_API void GLClearEx(f32 r, f32 g, f32 b, f32 a, i32 clearMask);

GFS_API void GLDrawElements(const GLElementBuffer *eb, const GLVertexBuffer *vb, GLVertexArray va);
GFS_API void GLDrawTriangles(const GLVertexBuffer *vb, const GLVertexBufferLayout *layout, GLVertexArray va);
GFS_API void GLDrawMesh(const Mesh *mesh);

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

GFS_API GLTexture GLTextureMakeFromBMPicture(const BMPicture *picture, GLTextureColorOrder colorOrder);

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
