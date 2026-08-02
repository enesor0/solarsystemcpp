#include "world/planet.h"

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

	glm::mat4 orbitOrientation(const PlanetDefinition& definition)
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

	orbitAngle_ = definition_.meanAnomalyAtEpoch;
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

void Planet::setOrbitElements(
	float longitudeOfAscendingNode,
	float argumentOfPeriapsis,
	float meanAnomalyAtEpoch)
{
	definition_.longitudeOfAscendingNode = longitudeOfAscendingNode;
	definition_.argumentOfPeriapsis = argumentOfPeriapsis;
	definition_.meanAnomalyAtEpoch = meanAnomalyAtEpoch;
	orbitAngle_ = meanAnomalyAtEpoch;
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

float Planet::orbitSpeed() const
{
	return definition_.orbitSpeed;
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

	glm::mat4 model = orbitOrientation(definition_);

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

	return glm::translate(orbitOrientation(definition_), orbitPosition);
}
