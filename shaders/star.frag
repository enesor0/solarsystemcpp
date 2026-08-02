#version 330 core

in vec3 starColor;

out vec4 fragmentColor;

void main()
{
    float distanceFromCenter = length(gl_PointCoord - vec2(0.5));

    if (distanceFromCenter > 0.5)
    {
        discard;
    }

    float intensity = 1.0 - smoothstep(0.0, 0.5, distanceFromCenter);
    fragmentColor = vec4(starColor * intensity, 1.0);
}
