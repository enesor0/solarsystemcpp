#version 330 core

in vec3 fragmentPosition;
in vec3 normal;
in vec2 textureCoords;

uniform vec3 objectColor;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 viewPosition;
uniform bool useTexture;
uniform sampler2D surfaceTexture;

out vec4 fragmentColor;

void main()
{
    vec3 normalizedNormal = normalize(normal);
    vec3 lightDirection = normalize(lightPosition - fragmentPosition);

    float diffuseStrength = max(
        dot(normalizedNormal, lightDirection),
        0.0
    );

    vec3 viewDirection = normalize(viewPosition - fragmentPosition);
    vec3 reflectionDirection = reflect(-lightDirection, normalizedNormal);

    float specularStrength = pow(
        max(dot(viewDirection, reflectionDirection), 0.0),
        32.0
    );

    vec3 albedo = objectColor;

    if (useTexture)
    {
        albedo = texture(surfaceTexture, textureCoords).rgb;
    }

    vec3 ambient = 0.08 * albedo;
    vec3 diffuse = diffuseStrength * albedo * lightColor;
    vec3 specular = 0.25 * specularStrength * lightColor;

    fragmentColor = vec4(ambient + diffuse + specular, 1.0);
}
