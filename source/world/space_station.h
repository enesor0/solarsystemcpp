#pragma once

#include <glm/glm.hpp>

#include <cstddef>
#include <string>

struct SpaceStationDefinition
{
	std::string name;
	std::size_t parentPlanetIndex = 0;
	float orbitRadius = 0.5f;
	float orbitSpeed = 0.0f;
	float orbitInclination = 0.0f;
	float scale = 0.05f;
};

class SpaceStation
{
public:
	explicit SpaceStation(SpaceStationDefinition definition);

	void update(float deltaTime);

	glm::mat4 modelMatrix(const glm::vec3& parentPosition) const;

	const std::string& name() const;
	std::size_t parentPlanetIndex() const;

private:
	SpaceStationDefinition definition_;
	float orbitAngle_ = 0.0f;
};
