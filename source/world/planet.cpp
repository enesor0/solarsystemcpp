#include "world/planet.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <stdexcept>
#include <utility>

Planet::Planet(PlanetDefinition definition)
	: definition_(std::move(definition))
{
	if (definition_.name.empty())
	{
		throw std::invalid_argument("Gezegen adi bos olamaz.");
	}

	if (definition_.radius <= 0.0f)
	{
		throw std::invalid_argument("Gezegen yaricapi pozitif olmalidir.");
	}

	if (definition_.orbitRadius < 0.0f)
	{
		throw std::invalid_argument("Yorunge yaricapi negatif olamaz.");
	}
}

void Planet::update(float deltaTime)
{
	selfRotationAngle_ = std::fmod(
		selfRotationAngle_ + definition_.selfRotationSpeed * deltaTime,
		360.0f
	);

	orbitAngle_ = std::fmod(
		orbitAngle_ + definition_.orbitSpeed * deltaTime,
		360.0f
	);
}

glm::mat4 Planet::modelMatrix() const
{
	glm::mat4 model(1.0f);

	model = glm::rotate(
		model,
		glm::radians(orbitAngle_),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	model = glm::translate(
		model,
		glm::vec3(definition_.orbitRadius, 0.0f, 0.0f)
	);

	model = glm::rotate(
		model,
		glm::radians(selfRotationAngle_),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	model = glm::scale(
		model,
		glm::vec3(definition_.radius)
	);

	return model;
}

const std::string& Planet::name() const
{
	return definition_.name;
}

const glm::vec3& Planet::baseColor() const
{
	return definition_.baseColor;
}