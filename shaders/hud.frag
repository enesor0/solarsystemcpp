#version 330 core

in vec4 color;
in vec2 textureCoordinates;
in float textMask;

uniform sampler2D fontTexture;

out vec4 fragmentColor;

void main()
{
    float fontOpacity = texture(fontTexture, textureCoordinates).r;
    float alpha = color.a * mix(1.0, fontOpacity, textMask);

    fragmentColor = vec4(color.rgb, alpha);
}
