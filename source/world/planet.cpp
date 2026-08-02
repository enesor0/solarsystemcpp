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

	if (definition_.orbitEccentricity < 0.0f
		|| definition_.orbitEccentricity >= 1.0f)
	{
		throw std::invalid_argument(
			"Yorunge eksantrikligi 0 ile 1 arasinda olmalidir."
		);
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
	glm::mat4 model = orbitalTransform();

	model = glm::rotate(
		model,
		glm::radians(definition_.axialTilt),
		glm::vec3(0.0f, 0.0f, 1.0f)
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

float Planet::radius() const
{
	return definition_.radius;
}

glm::vec3 Planet::position() const
{
	return glm::vec3(orbitalTransform()[3]);
}

float Planet::orbitRadius() const
{
	return definition_.orbitRadius;
}

bool Planet::emitsLight() const
{
	return definition_.emitsLight;
}

bool Planet::hasRing() const
{
	return definition_.ringOuterRadius > definition_.ringInnerRadius;
}

glm::mat4 Planet::ringModelMatrix() const
{
	glm::mat4 model = orbitalTransform();

	model = glm::rotate(
		model,
		glm::radians(definition_.axialTilt),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	model = glm::rotate(
		model,
		glm::radians(selfRotationAngle_),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	model = glm::rotate(
		model,
		glm::radians(definition_.ringTilt),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	return model;
}

glm::mat4 Planet::orbitPathModelMatrix() const
{
	const float semiMinorAxis = definition_.orbitRadius * std::sqrt(
		1.0f - definition_.orbitEccentricity * definition_.orbitEccentricity
	);

	glm::mat4 model(1.0f);

	model = glm::rotate(
		model,
		glm::radians(definition_.orbitInclination),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	model = glm::translate(
		model,
		glm::vec3(
			-definition_.orbitRadius * definition_.orbitEccentricity,
			0.0f,
			0.0f
		)
	);

	return glm::scale(
		model,
		glm::vec3(definition_.orbitRadius, 1.0f, semiMinorAxis)
	);
}

const glm::vec3& Planet::ringColor() const
{
	return definition_.ringColor;
}

glm::mat4 Planet::orbitalTransform() const
{
	const float orbitAngleRadians = glm::radians(orbitAngle_);
	const float semiMinorAxis = definition_.orbitRadius * std::sqrt(
		1.0f - definition_.orbitEccentricity * definition_.orbitEccentricity
	);

	const glm::vec3 orbitPosition(
		definition_.orbitRadius * (
			std::cos(orbitAngleRadians) - definition_.orbitEccentricity
		),
		0.0f,
		semiMinorAxis * std::sin(orbitAngleRadians)
	);

	glm::mat4 model(1.0f);

	model = glm::rotate(
		model,
		glm::radians(definition_.orbitInclination),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	model = glm::translate(
		model,
		orbitPosition
	);

	return model;
}
