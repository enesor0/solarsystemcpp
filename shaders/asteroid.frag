#version 330 core

in vec3 worldPosition;
in vec3 normal;
in vec3 color;

uniform vec3 lightPosition;
uniform vec3 viewPosition;

out vec4 fragmentColor;

vec3 toneMapACES(vec3 source)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp(
        (source * (a * source + b)) / (source * (c * source + d) + e),
        0.0,
        1.0
    );
}

void main()
{
    vec3 surfaceNormal = normalize(normal);
    vec3 lightDirection = normalize(lightPosition - worldPosition);
    vec3 viewDirection = normalize(viewPosition - worldPosition);
    vec3 halfVector = normalize(lightDirection + viewDirection);

    float diffuse = max(dot(surfaceNormal, lightDirection), 0.0);
    float specular = pow(max(dot(surfaceNormal, halfVector), 0.0), 28.0) * 0.10;
    vec3 lighting = color * (0.035 + diffuse * 1.7) + specular * vec3(0.92, 0.76, 0.55);
    vec3 mappedColor = pow(toneMapACES(lighting), vec3(1.0 / 2.2));

    fragmentColor = vec4(mappedColor, 1.0);
}
