#version 330 core

layout (location = 0) in vec2 l_Position;
layout (location = 1) in vec3 l_Color;

out vec4 f_Color;

uniform mat4 u_Model = mat4(0);
uniform mat4 u_Projection = mat4(0);

void main()
{
    vec4 position = u_Projection * u_Model * vec4(l_Position, 0.0, 1.0);
    gl_Position = vec4(position.xy, 0.0, 1.0);

    f_Color = vec4(l_Color, 1.0);
}
