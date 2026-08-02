#version 330 core

in vec3 worldPosition;
in vec3 normal;

uniform vec3 viewPosition;
uniform vec3 haloColor;
uniform float time;

out vec4 fragmentColor;

void main()
{
    vec3 viewDirection = normalize(viewPosition - worldPosition);
    float facing = max(dot(normalize(normal), viewDirection), 0.0);
    float rim = pow(1.0 - facing, 4.0);

    float pulse = 0.94 + 0.06 * sin(time * 2.0 + normal.y * 10.0);
    float alpha = rim * 0.16 * pulse;

    fragmentColor = vec4(haloColor * rim, alpha);
}
