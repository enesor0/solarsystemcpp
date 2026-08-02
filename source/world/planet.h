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

	glm::vec3 baseColor = glm::vec3(1.0f);
};

class Planet
{
public:
	explicit Planet(PlanetDefinition definition);

	void update(float deltaTime);

	glm::mat4 modelMatrix() const;

	const std::string& name() const;
	const glm::vec3& baseColor() const;

private:
	PlanetDefinition definition_;

	float selfRotationAngle_ = 0.0f;
	float orbitAngle_ = 0.0f;
};