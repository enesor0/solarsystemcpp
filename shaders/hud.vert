#version 330 core

layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec4 vertexColor;
layout (location = 2) in vec2 vertexTextureCoordinates;
layout (location = 3) in float vertexTextMask;

uniform mat4 projection;

out vec4 color;
out vec2 textureCoordinates;
out float textMask;

void main()
{
    color = vertexColor;
    textureCoordinates = vertexTextureCoordinates;
    textMask = vertexTextMask;
    gl_Position = projection * vec4(vertexPosition, 0.0, 1.0);
}
