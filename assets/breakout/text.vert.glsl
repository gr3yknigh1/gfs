#version 330 core

layout (location = 0) in vec2 l_Position;
layout (location = 1) in vec2 l_TexCoords;

out vec2 o_TexCoords;

uniform mat4 u_Projection = mat4(0);

void main() {
    vec4 position = u_Projection * mat4(1) * vec4(l_Position, 0.0, 1.0);
    gl_Position = vec4(position.xy, 0.0, 1.0);

    o_TexCoords = l_TexCoords;
}
