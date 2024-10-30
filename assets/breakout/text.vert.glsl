#version 330 core

layout (location = 0) in vec2 l_Position;
layout (location = 1) in vec2 l_TexCoords;;

out vec2 o_TexCoords;

uniform mat4 u_Projection;

void main() {
    gl_Position = u_Projection * vec4(l_Position, 0.0, 1.0);
    o_TexCoords = l_TexCoords;
}