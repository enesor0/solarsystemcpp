#pragma once

#include <glm/glm.hpp>

#include <cstddef>
#include <string>

struct MoonDefinition
{
	std::string name;
	std::size_t parentPlanetIndex = 0;

	float radius = 0.1f;
	float orbitRadius = 0.5f;
	float selfRotationSpeed = 0.0f;
	float orbitSpeed = 0.0f;
	float orbitInclination = 0.0f;
	float axialTilt = 0.0f;
	float orbitEccentricity = 0.0f;

	glm::vec3 baseColor = glm::vec3(1.0f);
	float longitudeOfAscendingNode = 0.0f;
	float argumentOfPeriapsis = 0.0f;
	float meanAnomalyAtEpoch = 0.0f;
};

class Moon
{
public:
	explicit Moon(MoonDefinition definition);

	void update(float deltaTime);
	void setOrbitElements(
		float longitudeOfAscendingNode,
		float argumentOfPeriapsis,
		float meanAnomalyAtEpoch
	);

	glm::mat4 modelMatrix(const glm::vec3& parentPosition) const;
	glm::mat4 orbitPathModelMatrix(const glm::vec3& parentPosition) const;

	const std::string& name() const;
	std::size_t parentPlanetIndex() const;
	float orbitRadius() const;
	const glm::vec3& baseColor() const;

private:
	MoonDefinition definition_;

	float selfRotationAngle_ = 0.0f;
	float orbitAngle_ = 0.0f;
};
