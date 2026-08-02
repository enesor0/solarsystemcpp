#include "world/space_station.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <stdexcept>
#include <utility>

SpaceStation::SpaceStation(SpaceStationDefinition definition)
	: definition_(std::move(definition))
{
	if (definition_.name.empty())
	{
		throw std::invalid_argument("Space station name cannot be empty.");
	}

	if (definition_.orbitRadius <= 0.0f || definition_.scale <= 0.0f)
	{
		throw std::invalid_argument(
			"Space station orbit radius and scale must be positive."
		);
	}
}

void SpaceStation::update(float deltaTime)
{
	orbitAngle_ = std::fmod(
		orbitAngle_ + definition_.orbitSpeed * deltaTime,
		360.0f
	);
}

glm::mat4 SpaceStation::modelMatrix(const glm::vec3& parentPosition) const
{
	const float orbitRadians = glm::radians(orbitAngle_);
	const glm::vec3 orbitPosition(
		std::cos(orbitRadians) * definition_.orbitRadius,
		0.0f,
		std::sin(orbitRadians) * definition_.orbitRadius
	);

	glm::mat4 model(1.0f);
	model = glm::translate(model, parentPosition);
	model = glm::rotate(
		model,
		glm::radians(definition_.orbitInclination),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
	model = glm::translate(model, orbitPosition);
	model = glm::rotate(
		model,
		-orbitRadians,
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	model = glm::rotate(
		model,
		glm::radians(23.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	return glm::scale(model, glm::vec3(definition_.scale));
}

const std::string& SpaceStation::name() const
{
	return definition_.name;
}

std::size_t SpaceStation::parentPlanetIndex() const
{
	return definition_.parentPlanetIndex;
}
