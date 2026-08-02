#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldPosition;
out vec3 normal;

void main()
{
    vec4 position = model * vec4(aPosition, 1.0);

    worldPosition = position.xyz;
    normal = normalize(mat3(transpose(inverse(model))) * aNormal);

    gl_Position = projection * view * position;
}
