#pragma once

#include "world/planet.h"

#include <vector>

class SolarSystem
{
public:
	SolarSystem();

	void update(float deltaTime);

	const std::vector<Planet>& planets() const;

private:
	std::vector<Planet> planets_;
};