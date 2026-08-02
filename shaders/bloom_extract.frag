#version 330 core

in vec2 textureCoords;

uniform sampler2D sceneTexture;

out vec4 fragmentColor;

void main()
{
    vec3 color = texture(sceneTexture, textureCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

    fragmentColor = brightness > 0.95 ? vec4(color, 1.0) : vec4(0.0);
}
