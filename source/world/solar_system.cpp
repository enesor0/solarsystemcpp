#include "world/solar_system.h"

SolarSystem::SolarSystem()
{
    planets_.reserve(9);

    planets_.emplace_back(PlanetDefinition{
        "Sun", 1.20f, 0.0f, 8.0f, 0.0f,
        glm::vec3(1.0f, 0.55f, 0.08f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Mercury", 0.12f, 2.00f, 25.0f, 65.0f,
        glm::vec3(0.55f, 0.50f, 0.45f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Venus", 0.20f, 2.80f, 18.0f, 48.0f,
        glm::vec3(0.90f, 0.67f, 0.34f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Earth", 0.22f, 3.70f, 80.0f, 38.0f,
        glm::vec3(0.10f, 0.35f, 1.00f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Mars", 0.17f, 4.60f, 65.0f, 30.0f,
        glm::vec3(0.80f, 0.16f, 0.08f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Jupiter", 0.55f, 6.10f, 45.0f, 18.0f,
        glm::vec3(0.85f, 0.55f, 0.32f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Saturn", 0.47f, 7.70f, 38.0f, 14.0f,
        glm::vec3(0.90f, 0.78f, 0.45f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Uranus", 0.33f, 9.20f, 30.0f, 10.0f,
        glm::vec3(0.38f, 0.80f, 0.88f)
    });

    planets_.emplace_back(PlanetDefinition{
        "Neptune", 0.32f, 10.70f, 28.0f, 8.0f,
        glm::vec3(0.12f, 0.30f, 0.95f)
    });
}

void SolarSystem::update(float deltaTime)
{
    for (Planet& planet : planets_)
    {
        planet.update(deltaTime);
    }
}

const std::vector<Planet>& SolarSystem::planets() const
{
    return planets_;
}
