#version 330 core

layout (location = 0) in vec3 l_Position;
// layout (location = 1) in vec3 l_Color;
layout (location = 2) in vec2 l_TexCoord;

out vec4 f_Color;
out vec2 f_TexCoord;

uniform mat4 u_Transform;
uniform float u_VertexModifier = 1;
uniform vec3 u_VertexOffset = vec3(0, 0, 0);

void main()
{
    gl_Position = u_Transform * vec4(
        l_Position * u_VertexModifier + u_VertexOffset, 1.0);

    f_Color = vec4(1);
    f_TexCoord = l_TexCoord;
}

