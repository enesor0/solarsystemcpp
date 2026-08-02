#version 330 core

uniform vec3 objectColor;

out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(objectColor, 1.0);
}