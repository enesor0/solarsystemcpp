#pragma once

#include <glm/glm.hpp>

#include <string>

struct PlanetDefinition
{
	std::string name;

	float radius = 1.0f;
	float orbitRadius = 0.0f;

	float selfRotationSpeed = 0.0f;
	float orbitSpeed = 0.0f;
	float orbitInclination = 0.0f;
	float axialTilt = 0.0f;
	float orbitEccentricity = 0.0f;

	glm::vec3 baseColor = glm::vec3(1.0f);
	bool emitsLight = false;

	float ringInnerRadius = 0.0f;
	float ringOuterRadius = 0.0f;
	float ringTilt = 0.0f;
	glm::vec3 ringColor = glm::vec3(1.0f);
};

class Planet
{
public:
	explicit Planet(PlanetDefinition definition);

	void update(float deltaTime);

	glm::mat4 modelMatrix() const;
	glm::mat4 orbitPathModelMatrix() const;

	const std::string& name() const;
	const glm::vec3& baseColor() const;
	float radius() const;
	glm::vec3 position() const;
	float orbitRadius() const;
	bool emitsLight() const;
	bool hasRing() const;
	glm::mat4 ringModelMatrix() const;
	const glm::vec3& ringColor() const;

private:
	glm::mat4 orbitalTransform() const;

	PlanetDefinition definition_;

	float selfRotationAngle_ = 0.0f;
	float orbitAngle_ = 0.0f;
};
