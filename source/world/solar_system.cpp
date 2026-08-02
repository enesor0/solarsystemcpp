#include "world/solar_system.h"

SolarSystem::SolarSystem()
{
	planets_.reserve(9);
	moons_.reserve(14);
	spaceStations_.reserve(1);

	planets_.emplace_back(PlanetDefinition{
		"Sun", 1.20f, 0.0f, 8.0f, 0.0f, 0.0f, 7.25f, 0.0f,
		glm::vec3(1.0f, 0.55f, 0.08f),
		true
	});

    planets_.emplace_back(PlanetDefinition{
        "Mercury", 0.12f, 2.30f, 25.0f, 65.0f, 7.00498f, 0.034f, 0.205636f,
        glm::vec3(0.55f, 0.50f, 0.45f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Venus", 0.20f, 3.15f, -18.0f, 48.0f, 3.39468f, 177.36f, 0.006777f,
        glm::vec3(0.90f, 0.67f, 0.34f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Earth", 0.22f, 3.70f, 80.0f, 38.0f, 0.00002f, 23.44f, 0.016711f,
        glm::vec3(0.10f, 0.35f, 1.00f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Mars", 0.17f, 4.57f, 65.0f, 30.0f, 1.84969f, 25.19f, 0.093394f,
        glm::vec3(0.80f, 0.16f, 0.08f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Jupiter", 0.55f, 8.44f, 45.0f, 18.0f, 1.30440f, 3.13f, 0.048386f,
        glm::vec3(0.85f, 0.55f, 0.32f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Saturn", 0.47f, 11.43f, 38.0f, 14.0f, 2.48599f, 26.73f, 0.053862f,
        glm::vec3(0.90f, 0.78f, 0.45f),
        false,
        0.62f, 1.00f, 0.0f,
        glm::vec3(0.76f, 0.64f, 0.42f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Uranus", 0.33f, 16.21f, -30.0f, 10.0f, 0.77264f, 97.77f, 0.047257f,
        glm::vec3(0.38f, 0.80f, 0.88f)
    });

	planets_.emplace_back(PlanetDefinition{
		"Neptune", 0.32f, 20.29f, 28.0f, 8.0f, 1.77004f, 28.32f, 0.008590f,
		glm::vec3(0.12f, 0.30f, 0.95f)
	});

	// J2000 osculating elements. Orbital radii are visually compressed so that
	// the complete system remains explorable in one scene; eccentricity,
	// orientation, epoch phase and period remain based on the real system.
	planets_.at(0).setOrbitElements(0.0f, 0.0f, 0.0f);
	planets_.at(1).setOrbitElements(48.33077f, 29.12703f, 174.79253f);
	planets_.at(2).setOrbitElements(76.67984f, 54.92262f, 50.37663f);
	planets_.at(3).setOrbitElements(0.0f, 102.93768f, 357.52689f);
	planets_.at(4).setOrbitElements(49.55954f, -73.50317f, 19.39020f);
	planets_.at(5).setOrbitElements(100.47391f, -85.74543f, 19.66796f);
	planets_.at(6).setOrbitElements(113.66242f, -21.06355f, -42.64463f);
	planets_.at(7).setOrbitElements(74.01693f, 96.93735f, 142.28383f);
	planets_.at(8).setOrbitElements(131.78423f, -86.81946f, -100.08479f);

	moons_.emplace_back(MoonDefinition{
		"Moon", 3,
		0.06f, 0.55f, 25.0f, 90.0f, 5.16f, 6.68f, 0.0554f,
		glm::vec3(0.72f, 0.72f, 0.72f)
	});

	moons_.emplace_back(MoonDefinition{
		"Phobos", 4,
		0.035f, 0.33f, 45.0f, 155.0f, 1.1f, 0.0f, 0.015f,
		glm::vec3(0.42f, 0.34f, 0.28f)
	});

	moons_.emplace_back(MoonDefinition{
		"Deimos", 4,
		0.025f, 0.48f, 35.0f, 105.0f, 1.8f, 0.9f, 0.000f,
		glm::vec3(0.56f, 0.47f, 0.38f)
	});

	moons_.emplace_back(MoonDefinition{
		"Io", 5,
		0.040f, 0.78f, 55.0f, 125.0f, 0.0f, 0.0f, 0.004f,
		glm::vec3(0.92f, 0.72f, 0.22f)
	});

	moons_.emplace_back(MoonDefinition{
		"Europa", 5,
		0.036f, 1.08f, 42.0f, 100.0f, 0.5f, 0.1f, 0.009f,
		glm::vec3(0.82f, 0.76f, 0.62f)
	});

	moons_.emplace_back(MoonDefinition{
		"Ganymede", 5,
		0.058f, 1.42f, 35.0f, 78.0f, 0.2f, 0.1f, 0.001f,
		glm::vec3(0.55f, 0.48f, 0.38f)
	});

	moons_.emplace_back(MoonDefinition{
		"Callisto", 5,
		0.052f, 1.80f, 30.0f, 60.0f, 0.3f, 0.4f, 0.007f,
		glm::vec3(0.32f, 0.28f, 0.24f)
	});

	moons_.emplace_back(MoonDefinition{
		"Enceladus", 6,
		0.020f, 1.12f, 55.0f, 110.0f, 0.0f, 0.0f, 0.005f,
		glm::vec3(0.84f, 0.90f, 0.96f)
	});

	moons_.emplace_back(MoonDefinition{
		"Titan", 6,
		0.058f, 1.48f, 40.0f, 52.0f, 0.3f, 0.6f, 0.029f,
		glm::vec3(0.78f, 0.48f, 0.16f)
	});

	moons_.emplace_back(MoonDefinition{
		"Rhea", 6,
		0.032f, 1.88f, 35.0f, 78.0f, 0.3f, 0.0f, 0.001f,
		glm::vec3(0.66f, 0.64f, 0.58f)
	});

	moons_.emplace_back(MoonDefinition{
		"Iapetus", 6,
		0.030f, 2.32f, 24.0f, 22.0f, 7.6f, 14.8f, 0.028f,
		glm::vec3(0.42f, 0.36f, 0.28f)
	});

	moons_.emplace_back(MoonDefinition{
		"Titania", 7,
		0.030f, 0.92f, 30.0f, 46.0f, 0.1f, 0.0f, 0.002f,
		glm::vec3(0.58f, 0.60f, 0.64f)
	});

	moons_.emplace_back(MoonDefinition{
		"Oberon", 7,
		0.028f, 1.22f, 28.0f, 30.0f, 0.1f, 0.0f, 0.002f,
		glm::vec3(0.38f, 0.36f, 0.34f)
	});

	moons_.emplace_back(MoonDefinition{
		"Triton", 8,
		0.035f, 0.98f, 45.0f, -60.0f, 157.3f, 0.4f, 0.000f,
		glm::vec3(0.70f, 0.78f, 0.84f)
	});

	// Mean orbital elements for the represented natural satellites.
	moons_.at(0).setOrbitElements(125.08f, 318.15f, 135.27f);
	moons_.at(1).setOrbitElements(169.20f, 216.30f, 189.70f);
	moons_.at(2).setOrbitElements(54.30f, 0.0f, 205.00f);
	moons_.at(3).setOrbitElements(0.0f, 49.10f, 330.90f);
	moons_.at(4).setOrbitElements(184.00f, 45.00f, 345.40f);
	moons_.at(5).setOrbitElements(58.50f, 198.30f, 324.80f);
	moons_.at(6).setOrbitElements(309.10f, 43.80f, 87.40f);
	moons_.at(7).setOrbitElements(0.0f, 119.50f, 57.00f);
	moons_.at(8).setOrbitElements(78.60f, 78.30f, 11.70f);
	moons_.at(9).setOrbitElements(133.70f, 44.30f, 31.50f);
	moons_.at(10).setOrbitElements(86.50f, 254.50f, 74.80f);
	moons_.at(11).setOrbitElements(29.50f, 184.00f, 68.10f);
	moons_.at(12).setOrbitElements(76.80f, 132.20f, 143.60f);
	moons_.at(13).setOrbitElements(178.10f, 0.0f, 63.00f);

	spaceStations_.emplace_back(SpaceStationDefinition{
		"International Space Station", 3,
		0.47f, 72.0f, 51.6f, 0.016f
	});
}

void SolarSystem::update(float deltaTime)
{
    for (Planet& planet : planets_)
    {
        planet.update(deltaTime);
    }

	for (Moon& moon : moons_)
	{
		moon.update(deltaTime);
	}

	for (SpaceStation& spaceStation : spaceStations_)
	{
		spaceStation.update(deltaTime);
	}
}

const std::vector<Planet>& SolarSystem::planets() const
{
    return planets_;
}

const std::vector<Moon>& SolarSystem::moons() const
{
	return moons_;
}

const std::vector<SpaceStation>& SolarSystem::spaceStations() const
{
	return spaceStations_;
}
