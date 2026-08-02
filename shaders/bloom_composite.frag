#version 330 core

in vec2 textureCoords;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;
uniform float exposure;
uniform float bloomStrength;

out vec4 fragmentColor;

void main()
{
    vec3 sceneColor = texture(sceneTexture, textureCoords).rgb;
    vec3 bloomColor = texture(bloomTexture, textureCoords).rgb;
    vec3 hdrColor = sceneColor + bloomColor * bloomStrength;

    vec3 toneMapped = vec3(1.0) - exp(-hdrColor * exposure);
    vec3 gammaCorrected = pow(toneMapped, vec3(1.0 / 2.2));

    fragmentColor = vec4(gammaCorrected, 1.0);
}
