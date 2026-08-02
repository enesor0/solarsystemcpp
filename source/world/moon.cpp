#include "world/moon.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <stdexcept>
#include <utility>

Moon::Moon(MoonDefinition definition)
	: definition_(std::move(definition))
{
	if (definition_.name.empty())
	{
		throw std::invalid_argument("Uydu adi bos olamaz.");
	}

	if (definition_.radius <= 0.0f || definition_.orbitRadius <= 0.0f)
	{
		throw std::invalid_argument(
			"Uydu yaricapi ve yorunge yaricapi pozitif olmalidir."
		);
	}

	if (definition_.orbitEccentricity < 0.0f
		|| definition_.orbitEccentricity >= 1.0f)
	{
		throw std::invalid_argument(
			"Uydu yorunge eksantrikligi 0 ile 1 arasinda olmalidir."
		);
	}
}

void Moon::update(float deltaTime)
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

glm::mat4 Moon::modelMatrix(const glm::vec3& parentPosition) const
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

	model = glm::translate(model, parentPosition);

	model = glm::rotate(
		model,
		glm::radians(definition_.orbitInclination),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	model = glm::translate(
		model,
		orbitPosition
	);

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

	return glm::scale(model, glm::vec3(definition_.radius));
}

glm::mat4 Moon::orbitPathModelMatrix(
	const glm::vec3& parentPosition) const
{
	const float semiMinorAxis = definition_.orbitRadius * std::sqrt(
		1.0f - definition_.orbitEccentricity * definition_.orbitEccentricity
	);

	glm::mat4 model(1.0f);

	model = glm::translate(model, parentPosition);

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

const std::string& Moon::name() const
{
	return definition_.name;
}

std::size_t Moon::parentPlanetIndex() const
{
	return definition_.parentPlanetIndex;
}

float Moon::orbitRadius() const
{
	return definition_.orbitRadius;
}

const glm::vec3& Moon::baseColor() const
{
	return definition_.baseColor;
}
