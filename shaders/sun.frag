#version 330 core

in vec3 worldPosition;
in vec3 normal;
in vec2 textureCoords;

uniform sampler2D sunTexture;
uniform float time;
uniform vec3 viewPosition;

out vec4 fragmentColor;

vec3 toneMapACES(vec3 color)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp(
        (color * (a * color + b)) / (color * (c * color + d) + e),
        0.0,
        1.0
    );
}

void main()
{
    float latitude = textureCoords.y * 2.0 - 1.0;
    vec2 animatedCoords = vec2(
        fract(textureCoords.x + time * 0.0018),
        clamp(
            textureCoords.y
                + sin(textureCoords.x * 19.0 + time * 0.18) * 0.0015,
            0.001,
            0.999
        )
    );

    vec3 sampledSurface = pow(
        texture(sunTexture, animatedCoords).rgb,
        vec3(2.2)
    );
    float luminance = dot(sampledSurface, vec3(0.2126, 0.7152, 0.0722));
    float convection = 0.90
        + 0.055 * sin(textureCoords.x * 53.0 + latitude * 17.0 + time * 0.65)
        + 0.035 * sin(textureCoords.x * 113.0 - latitude * 31.0 - time * 0.38);
    float activeRegion = smoothstep(0.48, 0.79, luminance) * 0.34;

    // Preserve the texture's granulation, but grade it from a deep amber
    // photosphere into a restrained warm-white active region.
    vec3 plasma = vec3(
        mix(0.44, 1.00, pow(sampledSurface.r, 0.74)),
        mix(0.025, 0.53, pow(sampledSurface.g, 0.92)),
        mix(0.002, 0.075, pow(sampledSurface.b, 1.45))
    );
    plasma *= convection;
    plasma = mix(plasma, vec3(1.0, 0.67, 0.26), activeRegion);

    vec3 viewDirection = normalize(viewPosition - worldPosition);
    float facing = max(dot(normalize(normal), viewDirection), 0.0);
    float limbDarkening = mix(0.58, 1.04, pow(facing, 0.48));
    float pulse = 0.975 + 0.025 * sin(time * 1.35 + normal.y * 7.0);
    vec3 emission = plasma * (1.48 * limbDarkening * pulse)
        + vec3(1.0, 0.38, 0.055) * activeRegion * 0.16;
    vec3 mappedColor = toneMapACES(emission);
    mappedColor = pow(mappedColor, vec3(1.0 / 2.2));

    fragmentColor = vec4(mappedColor, 1.0);
}
