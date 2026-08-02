#version 330 core

in vec3 worldPosition;
in vec3 normal;

uniform vec3 viewPosition;
uniform vec3 atmosphereColor;
uniform float intensity;

out vec4 fragmentColor;

void main()
{
    vec3 viewDirection = normalize(viewPosition - worldPosition);
    float facing = max(dot(normalize(normal), viewDirection), 0.0);
    float rim = pow(1.0 - facing, 3.8);

    fragmentColor = vec4(atmosphereColor * rim, rim * intensity);
}
