#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in mat4 instanceLocalModel;
layout (location = 5) in vec4 instanceOrbitData;
layout (location = 6) in vec4 instanceSpinData;
layout (location = 7) in vec4 instanceOrbitOrientation;
layout (location = 8) in vec3 instanceColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform int exclusionCount;
uniform vec3 exclusionCenters[9];
uniform float exclusionRadii[9];

out vec3 worldPosition;
out vec3 normal;
out vec3 color;

bool overlapsPlanetOnScreen(
    vec3 asteroidCenter,
    vec3 planetCenter,
    float planetRadius)
{
    vec4 planetViewPosition = view * vec4(planetCenter, 1.0);
    vec4 asteroidClipPosition = projection * view
        * vec4(asteroidCenter, 1.0);

    if (planetViewPosition.z >= -0.001 || asteroidClipPosition.w <= 0.0)
    {
        return false;
    }

    vec4 planetClipPosition = projection * planetViewPosition;
    vec2 screenOffset = asteroidClipPosition.xy / asteroidClipPosition.w
        - planetClipPosition.xy / planetClipPosition.w;
    vec2 screenRadius = vec2(projection[0][0], projection[1][1])
        * planetRadius / -planetViewPosition.z;
    vec2 normalizedOffset = screenOffset / max(
        screenRadius,
        vec2(0.0001)
    );

    return dot(normalizedOffset, normalizedOffset) < 1.0;
}

mat4 rotationAroundAxis(vec3 axis, float angle)
{
    float cosine = cos(angle);
    float sine = sin(angle);
    float inverseCosine = 1.0 - cosine;
    axis = normalize(axis);

    return mat4(
        cosine + axis.x * axis.x * inverseCosine,
        axis.x * axis.y * inverseCosine + axis.z * sine,
        axis.x * axis.z * inverseCosine - axis.y * sine,
        0.0,
        axis.y * axis.x * inverseCosine - axis.z * sine,
        cosine + axis.y * axis.y * inverseCosine,
        axis.y * axis.z * inverseCosine + axis.x * sine,
        0.0,
        axis.z * axis.x * inverseCosine + axis.y * sine,
        axis.z * axis.y * inverseCosine - axis.x * sine,
        cosine + axis.z * axis.z * inverseCosine,
        0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

float solveEccentricAnomaly(float meanAnomaly, float eccentricity)
{
    float eccentricAnomaly = meanAnomaly;

    for (int iteration = 0; iteration < 6; ++iteration)
    {
        eccentricAnomaly -= (
            eccentricAnomaly - eccentricity * sin(eccentricAnomaly)
            - meanAnomaly
        ) / (1.0 - eccentricity * cos(eccentricAnomaly));
    }

    return eccentricAnomaly;
}

mat4 asteroidOrbitOrientation()
{
    return rotationAroundAxis(
        vec3(0.0, 1.0, 0.0),
        radians(-instanceOrbitOrientation.y)
    ) * rotationAroundAxis(
        vec3(1.0, 0.0, 0.0),
        radians(-instanceOrbitOrientation.x)
    ) * rotationAroundAxis(
        vec3(0.0, 1.0, 0.0),
        radians(-instanceOrbitOrientation.z)
    );
}

void main()
{
    float meanAnomaly = instanceOrbitData.y + time * instanceOrbitData.z;
    float orbitRadius = instanceOrbitData.x;
    float eccentricity = instanceOrbitData.w;
    float eccentricAnomaly = solveEccentricAnomaly(meanAnomaly, eccentricity);
    float semiMinorAxis = orbitRadius * sqrt(1.0 - eccentricity * eccentricity);
    vec3 orbitPosition = vec3(
        orbitRadius * (cos(eccentricAnomaly) - eccentricity),
        0.0,
        semiMinorAxis * sin(eccentricAnomaly)
    );
    mat4 orbitModel = asteroidOrbitOrientation();
    orbitModel[3] = orbitModel * vec4(orbitPosition, 1.0);

    mat4 spinModel = rotationAroundAxis(
        instanceSpinData.xyz,
        time * instanceSpinData.w
    );
    mat4 worldModel = model * orbitModel * spinModel * instanceLocalModel;
    vec3 asteroidCenter = (worldModel * vec4(0.0, 0.0, 0.0, 1.0)).xyz;

    // Remove both real intersections and foreground asteroids that would cross a
    // planet's screen silhouette, keeping the belt visually behind each planet.
    for (int exclusionIndex = 0;
         exclusionIndex < exclusionCount;
         ++exclusionIndex)
    {
        if (distance(asteroidCenter, exclusionCenters[exclusionIndex])
                < exclusionRadii[exclusionIndex]
            || overlapsPlanetOnScreen(
                asteroidCenter,
                exclusionCenters[exclusionIndex],
                exclusionRadii[exclusionIndex]
            ))
        {
            gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
            return;
        }
    }

    vec4 worldPosition4 = worldModel * vec4(aPosition, 1.0);

    worldPosition = worldPosition4.xyz;
    normal = mat3(transpose(inverse(worldModel))) * aPosition;
    color = instanceColor;
    gl_Position = projection * view * worldPosition4;
}
