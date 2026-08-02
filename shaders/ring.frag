#version 330 core

in vec2 ringCoords;

uniform vec3 ringColor;

out vec4 fragmentColor;

void main()
{
    float radialPosition = ringCoords.y;

    float wideBands = 0.5 + 0.5 * sin(radialPosition * 42.0);
    float fineBands = 0.5 + 0.5 * sin(radialPosition * 176.0);
    float bandDensity = mix(wideBands, fineBands, 0.55);

    float innerFade = smoothstep(0.0, 0.07, radialPosition);
    float outerFade = 1.0 - smoothstep(0.90, 1.0, radialPosition);
    float alpha = (0.12 + 0.52 * bandDensity) * innerFade * outerFade;

    vec3 darkBand = ringColor * 0.38;
    vec3 lightBand = min(ringColor * 1.32, vec3(1.0));
    vec3 color = mix(darkBand, lightBand, bandDensity);

    fragmentColor = vec4(color, alpha);
}
