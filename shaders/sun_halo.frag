#version 330 core

in vec3 worldPosition;
in vec3 normal;

uniform vec3 viewPosition;
uniform vec3 haloColor;
uniform float haloIntensity;
uniform float time;

out vec4 fragmentColor;

void main()
{
    vec3 surfaceNormal = normalize(normal);
    vec3 viewDirection = normalize(viewPosition - worldPosition);
    float facing = max(dot(surfaceNormal, viewDirection), 0.0);
    float rim = pow(1.0 - facing, 3.05);

    float strandA = sin(
        surfaceNormal.x * 17.0 + surfaceNormal.z * 13.0 + time * 1.4
    );
    float strandB = sin(
        surfaceNormal.y * 23.0 - surfaceNormal.x * 11.0 - time * 0.8
    );
    float structure = 0.82 + 0.11 * strandA + 0.07 * strandB;
    float pulse = 0.97 + 0.03 * sin(time * 1.35 + surfaceNormal.y * 9.0);
    float alpha = rim * haloIntensity * structure * pulse;

    fragmentColor = vec4(haloColor * rim * structure, alpha);
}
