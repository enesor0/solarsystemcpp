#pragma once

#include "world/moon.h"
#include "world/planet.h"

#include <vector>

class SolarSystem
{
public:
	SolarSystem();

	void update(float deltaTime);

	const std::vector<Planet>& planets() const;
	const std::vector<Moon>& moons() const;

private:
	std::vector<Planet> planets_;
	std::vector<Moon> moons_;
};
