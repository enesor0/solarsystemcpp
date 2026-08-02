#version 330 core

in vec3 fragmentPosition;
in vec3 normal;
in vec2 textureCoords;

uniform vec3 objectColor;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 viewPosition;
uniform float roughness;
uniform float metallic;
uniform float surfaceDetailStrength;
uniform float nightLightIntensity;
uniform float time;
uniform float atmosphereFlowSpeed;
uniform float bandDetailStrength;
uniform float stormStrength;
uniform bool useTexture;
uniform sampler2D surfaceTexture;
uniform bool useNormalMap;
uniform sampler2D normalMap;
uniform bool useRoughnessMap;
uniform sampler2D roughnessMap;
uniform samplerCube shadowMap;
uniform float shadowFarPlane;
uniform bool shadowsEnabled;
uniform int shadowSampleCount;

out vec4 fragmentColor;

const float PI = 3.14159265359;

float distributionGGX(vec3 normal, vec3 halfVector, float materialRoughness)
{
    float alpha = materialRoughness * materialRoughness;
    float alphaSquared = alpha * alpha;
    float normalHalf = max(dot(normal, halfVector), 0.0);
    float normalHalfSquared = normalHalf * normalHalf;

    float denominator = normalHalfSquared * (alphaSquared - 1.0) + 1.0;
    return alphaSquared / max(PI * denominator * denominator, 0.0001);
}

float geometrySchlickGGX(float normalDirection, float materialRoughness)
{
    float roughnessTerm = materialRoughness + 1.0;
    float k = (roughnessTerm * roughnessTerm) / 8.0;

    return normalDirection / max(
        normalDirection * (1.0 - k) + k,
        0.0001
    );
}

float geometrySmith(
    vec3 normal,
    vec3 viewDirection,
    vec3 lightDirection,
    float materialRoughness)
{
    float normalView = max(dot(normal, viewDirection), 0.0);
    float normalLight = max(dot(normal, lightDirection), 0.0);

    return geometrySchlickGGX(normalView, materialRoughness)
        * geometrySchlickGGX(normalLight, materialRoughness);
}

vec3 fresnelSchlick(float cosine, vec3 reflectanceAtZero)
{
    return reflectanceAtZero
        + (1.0 - reflectanceAtZero) * pow(1.0 - cosine, 5.0);
}

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

float hash21(vec2 value)
{
    value = fract(value * vec2(123.34, 345.45));
    value += dot(value, value + 34.345);
    return fract(value.x * value.y);
}

float surfaceHeight(vec2 coordinates)
{
    vec3 color = texture(surfaceTexture, coordinates).rgb;
    return dot(color, vec3(0.299, 0.587, 0.114));
}

vec2 animatedSurfaceCoordinates()
{
    float latitude = textureCoords.y * 2.0 - 1.0;
    float jetStream = 0.60
        + 0.22 * sin(latitude * 21.0)
        + 0.12 * sin(latitude * 47.0 + 1.4);
    float flowOffset = time * atmosphereFlowSpeed * jetStream;

    return vec2(fract(textureCoords.x + flowOffset), textureCoords.y);
}

vec3 applySurfaceDetail(vec3 geometricNormal, vec2 materialCoordinates)
{
    if (!useTexture || surfaceDetailStrength <= 0.0)
    {
        return geometricNormal;
    }

    vec2 texelSize = 1.0 / vec2(textureSize(surfaceTexture, 0));
    float left = surfaceHeight(materialCoordinates - vec2(texelSize.x, 0.0));
    float right = surfaceHeight(materialCoordinates + vec2(texelSize.x, 0.0));
    float down = surfaceHeight(materialCoordinates - vec2(0.0, texelSize.y));
    float up = surfaceHeight(materialCoordinates + vec2(0.0, texelSize.y));

    vec3 referenceAxis = abs(geometricNormal.y) < 0.97
        ? vec3(0.0, 1.0, 0.0)
        : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(referenceAxis, geometricNormal));
    vec3 bitangent = normalize(cross(geometricNormal, tangent));
    vec3 detailNormal = geometricNormal
        - tangent * (right - left) * surfaceDetailStrength
        - bitangent * (up - down) * surfaceDetailStrength;

    return normalize(detailNormal);
}

vec3 applyNormalMap(vec3 geometricNormal, vec2 materialCoordinates)
{
    if (!useNormalMap)
    {
        return applySurfaceDetail(geometricNormal, materialCoordinates);
    }

    vec3 sampledNormal = texture(normalMap, materialCoordinates).rgb * 2.0 - 1.0;
    vec3 referenceAxis = abs(geometricNormal.y) < 0.97
        ? vec3(0.0, 1.0, 0.0)
        : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(referenceAxis, geometricNormal));
    vec3 bitangent = normalize(cross(geometricNormal, tangent));

    return normalize(
        tangent * sampledNormal.x
        + bitangent * sampledNormal.y
        + geometricNormal * sampledNormal.z
    );
}

float shadowAmount(
    vec3 worldPosition,
    vec3 surfaceNormal,
    vec3 lightDirection)
{
	if (!shadowsEnabled || shadowSampleCount <= 0)
	{
		return 0.0;
	}

    vec3 fragmentToLight = worldPosition - lightPosition;
    float currentDepth = length(fragmentToLight);
    float bias = max(0.035 * (1.0 - dot(surfaceNormal, lightDirection)), 0.008);
    float shadow = 0.0;
    float sampleRadius = 0.035 + currentDepth * 0.006;

    const vec3 sampleOffsets[12] = vec3[](
        vec3(1.0, 1.0, 1.0), vec3(-1.0, -1.0, 1.0),
        vec3(-1.0, 1.0, -1.0), vec3(1.0, -1.0, -1.0),
        vec3(1.0, 1.0, -1.0), vec3(-1.0, 1.0, 1.0),
        vec3(1.0, -1.0, 1.0), vec3(-1.0, -1.0, -1.0),
        vec3(1.0, 0.0, 0.0), vec3(-1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0), vec3(0.0, -1.0, 0.0)
    );

    for (int sampleIndex = 0; sampleIndex < 12; ++sampleIndex)
    {
		if (sampleIndex >= shadowSampleCount)
		{
			break;
		}

        float sampledDepth = texture(
            shadowMap,
            fragmentToLight + sampleOffsets[sampleIndex] * sampleRadius
        ).r * shadowFarPlane;

        shadow += currentDepth - bias > sampledDepth ? 1.0 : 0.0;
    }

    return shadow / float(shadowSampleCount);
}

void main()
{
    vec2 materialCoordinates = animatedSurfaceCoordinates();
    vec3 sampledAlbedo = objectColor;

    if (useTexture)
    {
        sampledAlbedo = texture(surfaceTexture, materialCoordinates).rgb;
    }

    if (bandDetailStrength > 0.0)
    {
        float latitude = materialCoordinates.y * 2.0 - 1.0;
        float broadBands = sin(latitude * 38.0 + sin(materialCoordinates.x * 9.0));
        float fineBands = sin(latitude * 98.0 - materialCoordinates.x * 4.0);
        float bandLighting = 1.0
            + broadBands * bandDetailStrength * 0.42
            + fineBands * bandDetailStrength * 0.16;
        sampledAlbedo *= bandLighting;

        float wrapDistance = abs(materialCoordinates.x - 0.68);
        wrapDistance = min(wrapDistance, 1.0 - wrapDistance);
        vec2 stormDistance = vec2(
            wrapDistance / 0.105,
            (materialCoordinates.y - 0.585) / 0.045
        );
        float stormMask = 1.0 - smoothstep(
            0.72,
            1.0,
            dot(stormDistance, stormDistance)
        );
        vec3 stormColor = vec3(0.72, 0.19, 0.075);
        sampledAlbedo = mix(
            sampledAlbedo,
            stormColor,
            stormMask * stormStrength * 0.68
        );
    }

    vec3 albedo = pow(sampledAlbedo, vec3(2.2));

    vec3 normalizedNormal = applyNormalMap(
        normalize(normal),
        materialCoordinates
    );
    float materialRoughness = roughness;

    if (useRoughnessMap)
    {
        materialRoughness = texture(roughnessMap, materialCoordinates).r;
    }

    materialRoughness = clamp(materialRoughness, 0.06, 0.98);
    vec3 viewDirection = normalize(viewPosition - fragmentPosition);
    vec3 lightDirection = normalize(lightPosition - fragmentPosition);
    vec3 halfVector = normalize(viewDirection + lightDirection);

    float normalLight = max(dot(normalizedNormal, lightDirection), 0.0);
    vec3 reflectanceAtZero = mix(vec3(0.04), albedo, metallic);
    vec3 fresnel = fresnelSchlick(
        max(dot(halfVector, viewDirection), 0.0),
        reflectanceAtZero
    );
    float distribution = distributionGGX(
        normalizedNormal,
        halfVector,
        materialRoughness
    );
    float geometry = geometrySmith(
        normalizedNormal,
        viewDirection,
        lightDirection,
        materialRoughness
    );
    vec3 specular = (distribution * geometry * fresnel)
        / max(
            4.0 * max(dot(normalizedNormal, viewDirection), 0.0) * normalLight,
            0.0001
        );

    vec3 specularRatio = fresnel;
    vec3 diffuseRatio = (vec3(1.0) - specularRatio) * (1.0 - metallic);
    vec3 directLighting = (
        diffuseRatio * albedo / PI + specular
    ) * lightColor * normalLight;
    float shadow = shadowAmount(
        fragmentPosition,
        normalizedNormal,
        lightDirection
    );
    vec3 ambientLighting = albedo * 0.018;

    float waterMask = smoothstep(
        0.04,
        0.22,
        sampledAlbedo.b - max(sampledAlbedo.r, sampledAlbedo.g) * 0.68
    );
    float landMask = 1.0 - waterMask;
    float cityPattern = smoothstep(
        0.945,
        0.992,
        hash21(floor(textureCoords * vec2(420.0, 210.0)))
    );
    float nightSide = 1.0 - smoothstep(-0.28, 0.12, normalLight);
    vec3 cityLights = vec3(1.0, 0.48, 0.12)
        * cityPattern * landMask * nightSide * nightLightIntensity;

    vec3 mappedColor = toneMapACES(
        ambientLighting + directLighting * (1.0 - shadow) + cityLights
    );
    mappedColor = pow(mappedColor, vec3(1.0 / 2.2));

    fragmentColor = vec4(mappedColor, 1.0);
}
