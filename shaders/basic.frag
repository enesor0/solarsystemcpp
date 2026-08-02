#version 330 core

uniform vec3 objectColor;
uniform float opacity;

out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(objectColor, opacity);
}
