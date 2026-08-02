#version 330 core

in vec3 worldPosition;
in vec3 normal;

uniform vec3 viewPosition;
uniform vec3 lightPosition;
uniform vec3 atmosphereColor;
uniform float intensity;

out vec4 fragmentColor;

void main()
{
    vec3 surfaceNormal = normalize(normal);
    vec3 viewDirection = normalize(viewPosition - worldPosition);
    vec3 lightDirection = normalize(lightPosition - worldPosition);

    float facing = max(dot(surfaceNormal, viewDirection), 0.0);
    float rim = pow(1.0 - facing, 3.65);
    float lightFacing = dot(surfaceNormal, lightDirection);
    float daySide = smoothstep(-0.38, 0.42, lightFacing);
    float forwardScatter = pow(
        max(dot(-lightDirection, viewDirection), 0.0),
        7.0
    );

    float scattering = rim * mix(0.18, 1.0, daySide)
        + forwardScatter * 0.22;
    float alpha = scattering * intensity;
    vec3 color = atmosphereColor * (0.55 + 0.45 * daySide);

    fragmentColor = vec4(color * scattering, alpha);
}
