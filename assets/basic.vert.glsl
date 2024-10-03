#version 330 core

layout (location = 0) in vec3 l_Position;
layout (location = 1) in vec3 l_Color;
layout (location = 2) in vec2 l_TexCoord;

out vec4 f_Color;
out vec2 f_TexCoord;

uniform mat4 u_Model = mat4(0);
uniform mat4 u_View = mat4(0);
uniform mat4 u_Projection = mat4(0);

uniform float u_VertexModifier = 1;
uniform vec3 u_VertexOffset = vec3(0, 0, 0);

void main()
{
    mat4 transformation =  u_Projection * u_View * u_Model;
    gl_Position = transformation * vec4(
        l_Position * u_VertexModifier + u_VertexOffset, 1.0);

    f_Color = vec4(l_Color, 1.0);
    f_TexCoord = l_TexCoord;
}

