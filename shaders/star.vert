#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in float aSize;

uniform mat4 view;
uniform mat4 projection;

out vec3 starColor;

void main()
{
    mat4 rotationView = mat4(mat3(view));

    gl_Position = projection * rotationView * vec4(aPosition, 1.0);
    gl_PointSize = aSize;
    starColor = aColor;
}
