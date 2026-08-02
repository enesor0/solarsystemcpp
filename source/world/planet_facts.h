#pragma once

#include <string_view>

struct PlanetFacts
{
	std::string_view summary;
	unsigned int diameterKilometers = 0;
	float averageDistanceMillionKm = 0.0f;
};

const PlanetFacts& planetFacts(std::string_view planetName);
