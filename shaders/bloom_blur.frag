#version 330 core

in vec2 textureCoords;

uniform sampler2D sourceTexture;
uniform bool horizontal;

out vec4 fragmentColor;

void main()
{
    float weights[5] = float[](
        0.227027,
        0.1945946,
        0.1216216,
        0.054054,
        0.016216
    );

    vec2 texelOffset = 1.0 / vec2(textureSize(sourceTexture, 0));
    vec3 result = texture(sourceTexture, textureCoords).rgb * weights[0];

    for (int index = 1; index < 5; ++index)
    {
        vec2 offset = horizontal
            ? vec2(texelOffset.x * index, 0.0)
            : vec2(0.0, texelOffset.y * index);

        result += texture(sourceTexture, textureCoords + offset).rgb * weights[index];
        result += texture(sourceTexture, textureCoords - offset).rgb * weights[index];
    }

    fragmentColor = vec4(result, 1.0);
}
