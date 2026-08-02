#version 330 core

in vec4 fragmentWorldPosition;

uniform vec3 lightPosition;
uniform float farPlane;

void main()
{
    float lightDistance = length(fragmentWorldPosition.xyz - lightPosition);
    gl_FragDepth = lightDistance / farPlane;
}
