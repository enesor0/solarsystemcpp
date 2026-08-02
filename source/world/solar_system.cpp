#include "world/solar_system.h"

SolarSystem::SolarSystem()
{
    planets_.reserve(9);
	moons_.reserve(7);

	planets_.emplace_back(PlanetDefinition{
		"Sun", 1.20f, 0.0f, 8.0f, 0.0f, 0.0f, 7.25f, 0.0f,
		glm::vec3(1.0f, 0.55f, 0.08f),
		true
	});

    planets_.emplace_back(PlanetDefinition{
        "Mercury", 0.12f, 2.00f, 25.0f, 65.0f, 7.0f, 0.03f, 0.205f,
        glm::vec3(0.55f, 0.50f, 0.45f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Venus", 0.20f, 2.80f, 18.0f, 48.0f, 3.4f, 177.4f, 0.007f,
        glm::vec3(0.90f, 0.67f, 0.34f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Earth", 0.22f, 3.70f, 80.0f, 38.0f, 0.0f, 23.4f, 0.017f,
        glm::vec3(0.10f, 0.35f, 1.00f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Mars", 0.17f, 4.60f, 65.0f, 30.0f, 1.8f, 25.2f, 0.093f,
        glm::vec3(0.80f, 0.16f, 0.08f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Jupiter", 0.55f, 6.10f, 45.0f, 18.0f, 1.3f, 3.1f, 0.049f,
        glm::vec3(0.85f, 0.55f, 0.32f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Saturn", 0.47f, 7.70f, 38.0f, 14.0f, 2.5f, 26.7f, 0.057f,
        glm::vec3(0.90f, 0.78f, 0.45f),
        false,
        0.62f, 1.00f, 0.0f,
        glm::vec3(0.76f, 0.64f, 0.42f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Uranus", 0.33f, 9.20f, 30.0f, 10.0f, 0.8f, 97.8f, 0.046f,
        glm::vec3(0.38f, 0.80f, 0.88f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Neptune", 0.32f, 10.70f, 28.0f, 8.0f, 1.8f, 28.3f, 0.010f,
        glm::vec3(0.12f, 0.30f, 0.95f)
    });

	moons_.emplace_back(MoonDefinition{
		"Moon", 3,
		0.06f, 0.55f, 25.0f, 90.0f, 5.1f, 6.68f, 0.055f,
		glm::vec3(0.72f, 0.72f, 0.72f)
	});

	moons_.emplace_back(MoonDefinition{
		"Phobos", 4,
		0.035f, 0.33f, 45.0f, 155.0f, 1.1f, 0.0f, 0.015f,
		glm::vec3(0.42f, 0.34f, 0.28f)
	});

	moons_.emplace_back(MoonDefinition{
		"Deimos", 4,
		0.025f, 0.48f, 35.0f, 105.0f, 1.8f, 0.0f, 0.000f,
		glm::vec3(0.56f, 0.47f, 0.38f)
	});

	moons_.emplace_back(MoonDefinition{
		"Io", 5,
		0.060f, 0.78f, 55.0f, 125.0f, 0.1f, 0.0f, 0.004f,
		glm::vec3(0.92f, 0.72f, 0.22f)
	});

	moons_.emplace_back(MoonDefinition{
		"Europa", 5,
		0.050f, 1.08f, 42.0f, 100.0f, 0.5f, 0.1f, 0.009f,
		glm::vec3(0.82f, 0.76f, 0.62f)
	});

	moons_.emplace_back(MoonDefinition{
		"Ganymede", 5,
		0.085f, 1.42f, 35.0f, 78.0f, 0.3f, 0.0f, 0.001f,
		glm::vec3(0.55f, 0.48f, 0.38f)
	});

	moons_.emplace_back(MoonDefinition{
		"Callisto", 5,
		0.075f, 1.80f, 30.0f, 60.0f, 0.2f, 0.0f, 0.007f,
		glm::vec3(0.32f, 0.28f, 0.24f)
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
}

const std::vector<Planet>& SolarSystem::planets() const
{
    return planets_;
}

const std::vector<Moon>& SolarSystem::moons() const
{
	return moons_;
}
