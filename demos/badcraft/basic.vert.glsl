#version 330 core

layout (location = 0) in vec3 l_Position;
layout (location = 1) in vec4 l_Color;

out vec4 f_Color;

void main()
{
    gl_Position = vec4(l_Position, 1.0);
    f_Color = l_Color;
}

