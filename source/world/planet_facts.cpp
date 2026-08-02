#include "world/planet_facts.h"

const PlanetFacts& planetFacts(std::string_view planetName)
{
	static const PlanetFacts sun{
		"THE SYSTEM'S CENTRAL STAR AND LIGHT SOURCE", 1392700, 0.0f
	};
	static const PlanetFacts mercury{
		"THE CLOSEST ROCKY PLANET TO THE SUN", 4879, 57.9f
	};
	static const PlanetFacts venus{
		"A ROCKY PLANET WITH A DENSE ATMOSPHERE", 12104, 108.2f
	};
	static const PlanetFacts earth{
		"THE BLUE PLANET KNOWN FOR LIQUID WATER", 12742, 149.6f
	};
	static const PlanetFacts mars{
		"THE RED PLANET, RICH IN IRON OXIDE", 6779, 227.9f
	};
	static const PlanetFacts jupiter{
		"THE LARGEST GAS GIANT IN THE SOLAR SYSTEM", 139820, 778.6f
	};
	static const PlanetFacts saturn{
		"FAMOUS FOR ITS ICY, ROCKY RING SYSTEM", 116460, 1433.5f
	};
	static const PlanetFacts uranus{
		"AN ICE GIANT WITH AN EXTREME AXIAL TILT", 50724, 2872.5f
	};
	static const PlanetFacts neptune{
		"A WINDY ICE GIANT IN THE OUTER SYSTEM", 49244, 4495.1f
	};
	static const PlanetFacts unknown{
		"NO ADDITIONAL INFORMATION AVAILABLE", 0, 0.0f
	};

	if (planetName == "Sun") return sun;
	if (planetName == "Mercury") return mercury;
	if (planetName == "Venus") return venus;
	if (planetName == "Earth") return earth;
	if (planetName == "Mars") return mars;
	if (planetName == "Jupiter") return jupiter;
	if (planetName == "Saturn") return saturn;
	if (planetName == "Uranus") return uranus;
	if (planetName == "Neptune") return neptune;

	return unknown;
}
