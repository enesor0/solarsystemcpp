#version 330 core

in vec3 worldPosition;
in vec3 normal;
in vec2 textureCoords;

uniform sampler2D sunTexture;
uniform float time;

out vec4 fragmentColor;

void main()
{
    vec2 animatedCoords = textureCoords;
    animatedCoords.x += time * 0.006;

    vec3 surfaceColor = texture(sunTexture, animatedCoords).rgb;
    float pulse = 1.05 + 0.04 * sin(time * 2.0 + normal.y * 8.0);

    fragmentColor = vec4(surfaceColor * pulse * 1.15, 1.0);
}
