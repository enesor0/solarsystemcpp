#include "world/moon.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <stdexcept>
#include <utility>

namespace
{
	float solveEccentricAnomaly(float meanAnomaly, float eccentricity)
	{
		float eccentricAnomaly = meanAnomaly;

		for (int iteration = 0; iteration < 8; ++iteration)
		{
			const float correction = (
				eccentricAnomaly - eccentricity * std::sin(eccentricAnomaly)
				- meanAnomaly
			) / (1.0f - eccentricity * std::cos(eccentricAnomaly));
			eccentricAnomaly -= correction;
		}

		return eccentricAnomaly;
	}

	glm::mat4 orbitOrientation(const MoonDefinition& definition)
	{
		glm::mat4 orientation(1.0f);
		orientation = glm::rotate(
			orientation,
			glm::radians(-definition.longitudeOfAscendingNode),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
		orientation = glm::rotate(
			orientation,
			glm::radians(-definition.orbitInclination),
			glm::vec3(1.0f, 0.0f, 0.0f)
		);
		return glm::rotate(
			orientation,
			glm::radians(-definition.argumentOfPeriapsis),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
	}
}

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

	orbitAngle_ = definition_.meanAnomalyAtEpoch;
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

void Moon::setOrbitElements(
	float longitudeOfAscendingNode,
	float argumentOfPeriapsis,
	float meanAnomalyAtEpoch)
{
	definition_.longitudeOfAscendingNode = longitudeOfAscendingNode;
	definition_.argumentOfPeriapsis = argumentOfPeriapsis;
	definition_.meanAnomalyAtEpoch = meanAnomalyAtEpoch;
	orbitAngle_ = meanAnomalyAtEpoch;
}

glm::mat4 Moon::modelMatrix(const glm::vec3& parentPosition) const
{
	const float meanAnomaly = glm::radians(orbitAngle_);
	const float eccentricAnomaly = solveEccentricAnomaly(
		meanAnomaly,
		definition_.orbitEccentricity
	);
	const float semiMinorAxis = definition_.orbitRadius * std::sqrt(
		1.0f - definition_.orbitEccentricity * definition_.orbitEccentricity
	);

	const glm::vec3 orbitPosition(
		definition_.orbitRadius * (
			std::cos(eccentricAnomaly) - definition_.orbitEccentricity
		),
		0.0f,
		semiMinorAxis * std::sin(eccentricAnomaly)
	);

	glm::mat4 model(1.0f);

	model = glm::translate(model, parentPosition);
	model *= orbitOrientation(definition_);
	model = glm::translate(model, orbitPosition);

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
	model *= orbitOrientation(definition_);

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
