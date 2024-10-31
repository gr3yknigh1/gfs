#version 330 core

in vec2 o_TexCoords;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec4 u_Color;

void main() {
    FragColor = vec4(1, 0, 0, 1); // u_Color; // * vec4(1.0, 1.0, 1.0, texture(u_Texture, o_TexCoords).r);
}
