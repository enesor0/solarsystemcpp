#version 330 core

in vec3 worldPosition;
in vec3 normal;
in vec2 textureCoords;

uniform sampler2D earthTexture;
uniform vec3 lightPosition;
uniform vec3 viewPosition;
uniform float time;
uniform float cloudOpacity;
uniform float cloudLayerOffset;
uniform float cloudCoverage;

out vec4 fragmentColor;

void main()
{
    vec2 cloudCoordinates = vec2(
        fract(textureCoords.x + time * 0.0018 + cloudLayerOffset),
        clamp(
            textureCoords.y
                + sin(textureCoords.x * 29.0 + time * 0.12) * 0.0012,
            0.001,
            0.999
        )
    );

    vec3 sourceColor = texture(earthTexture, cloudCoordinates).rgb;
    float brightestChannel = max(max(sourceColor.r, sourceColor.g), sourceColor.b);
    float darkestChannel = min(min(sourceColor.r, sourceColor.g), sourceColor.b);
    float saturation = brightestChannel - darkestChannel;
    float brightMask = smoothstep(0.60, 0.90, brightestChannel);
    float neutralMask = 1.0 - smoothstep(0.12, 0.42, saturation);
    float polarIceReduction = 1.0 - smoothstep(
        0.84,
        0.98,
        abs(textureCoords.y * 2.0 - 1.0)
    ) * 0.36;
    float cloudMask = brightMask * neutralMask * polarIceReduction * cloudCoverage;

    if (cloudMask < 0.05)
    {
        discard;
    }

    vec3 lightDirection = normalize(lightPosition - worldPosition);
    float illumination = max(dot(normalize(normal), lightDirection), 0.0);
    vec3 viewDirection = normalize(viewPosition - worldPosition);
    float facing = max(dot(normalize(normal), viewDirection), 0.0);
    float horizonGlow = pow(1.0 - facing, 2.2) * 0.20;
    vec3 cloudColor = mix(
        vec3(0.34, 0.40, 0.52),
        vec3(0.96, 0.98, 1.0),
        clamp(illumination + horizonGlow, 0.0, 1.0)
    );

    fragmentColor = vec4(
        cloudColor,
        cloudMask * 0.38 * cloudOpacity * (0.22 + 0.78 * illumination)
    );
}
