//
// FILE     assets/chunk.vert.glsl
//

#version 330 core

// @breaf Location of geometry in world space.
layout (location = 0) in vec3 l_Position;

// @breaf Texture coordinates.
layout (location = 1) in vec2 l_UV;

// @breaf Model matrix.
uniform mat4 u_Model = mat4(0);

// @breaf View matrix.
uniform mat4 u_View = mat4(0);

// @breaf Projection matrix.
uniform mat4 u_Projection = mat4(0);

out vec2 f_UV;

void main() {
    gl_Position = u_Projection * u_View * u_Model * vec4(l_Position, 1.0);
    f_UV = l_UV;
}