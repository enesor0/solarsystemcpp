#include "graphics/ring.h"

#include <cmath>
#include <cstddef>
#include <stdexcept>

Ring::Ring(
	float innerRadius,
	float outerRadius,
	unsigned int sectorCount)
	: mesh_(createMeshData(innerRadius, outerRadius, sectorCount))
{
}

void Ring::draw() const
{
	mesh_.draw();
}

MeshData Ring::createMeshData(
	float innerRadius,
	float outerRadius,
	unsigned int sectorCount)
{
	if (innerRadius <= 0.0f || outerRadius <= innerRadius || sectorCount < 3)
	{
		throw std::invalid_argument(
			"Halka yaricaplari pozitif olmali ve dis yaricap daha buyuk olmalidir."
		);
	}

	MeshData data;

	const unsigned int verticesPerRing = sectorCount + 1;

	data.vertices.reserve(
		static_cast<std::size_t>(verticesPerRing) * 2
	);
	data.indices.reserve(static_cast<std::size_t>(sectorCount) * 6);

	constexpr float pi = 3.14159265359f;

	for (unsigned int sector = 0; sector <= sectorCount; ++sector)
	{
		const float ratio =
			static_cast<float>(sector) / static_cast<float>(sectorCount);
		const float angle = ratio * 2.0f * pi;

		const float x = std::cos(angle);
		const float z = std::sin(angle);

		data.vertices.push_back(Vertex{
			glm::vec3(innerRadius * x, 0.0f, innerRadius * z),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec2(ratio, 0.0f)
		});

		data.vertices.push_back(Vertex{
			glm::vec3(outerRadius * x, 0.0f, outerRadius * z),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec2(ratio, 1.0f)
		});
	}

	for (unsigned int sector = 0; sector < sectorCount; ++sector)
	{
		const unsigned int currentInner = sector * 2;
		const unsigned int currentOuter = currentInner + 1;
		const unsigned int nextInner = currentInner + 2;
		const unsigned int nextOuter = currentInner + 3;

		data.indices.push_back(currentInner);
		data.indices.push_back(nextInner);
		data.indices.push_back(currentOuter);

		data.indices.push_back(currentOuter);
		data.indices.push_back(nextInner);
		data.indices.push_back(nextOuter);
	}

	return data;
}
