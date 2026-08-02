#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldPosition;
out vec3 normal;
out vec2 textureCoords;

void main()
{
    vec4 position = model * vec4(aPosition, 1.0);

    worldPosition = position.xyz;
    normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    textureCoords = aTextureCoords;

    gl_Position = projection * view * position;
}
