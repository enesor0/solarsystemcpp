#include "graphics/sphere.h"

#include <cstddef>
#include <cmath>
#include <stdexcept>

Sphere::Sphere(
	float radius,
	unsigned int sectorCount,
	unsigned int stackCount)
	: mesh_(createMeshData(radius, sectorCount, stackCount))
{
}

void Sphere::draw() const
{
	mesh_.draw();
}

MeshData Sphere::createMeshData(
	float radius,
	unsigned int sectorCount,
	unsigned int stackCount)
{
	if (radius <= 0.0f || sectorCount < 3 || stackCount < 2)
	{
		throw std::invalid_argument(
			"Kure yaricapi pozitif, sector sayisi en az 3 ve stack sayisi en az 2 olmalidir."
		);
	}

	constexpr float pi = 3.14159265358979323846f;

	MeshData data;

	const unsigned int verticesPerRow = sectorCount + 1;

	data.vertices.reserve(
		static_cast<std::size_t>(stackCount + 1) * verticesPerRow
	);

	data.indices.reserve(
		static_cast<std::size_t>(stackCount) * sectorCount * 6
	);

	for (unsigned int stack = 0; stack <= stackCount; ++stack)
	{
		const float stackRatio =
			static_cast<float>(stack) / static_cast<float>(stackCount);

		const float phi = stackRatio * pi;
		const float y = radius * std::cos(phi);
		const float ringRadius = radius * std::sin(phi);

		for (unsigned int sector = 0; sector <= sectorCount; ++sector)
		{
			const float sectorRatio =
				static_cast<float>(sector)
				/ static_cast<float>(sectorCount);

			const float theta = sectorRatio * 2.0f * pi;

			const glm::vec3 position(
				ringRadius * std::cos(theta),
				y,
				ringRadius * std::sin(theta)
			);

			const glm::vec3 normal = glm::normalize(position);

			const glm::vec2 textureCoords(
				sectorRatio,
				1.0f - stackRatio
			);

			data.vertices.push_back(
				Vertex{ position, normal, textureCoords }
			);
		}
	}

	for (unsigned int stack = 0; stack < stackCount; ++stack)
	{
		for (unsigned int sector = 0; sector < sectorCount; ++sector)
		{
			const unsigned int first =
				stack * verticesPerRow + sector;

			const unsigned int second =
				first + verticesPerRow;

			if (stack != 0)
			{
				data.indices.push_back(first);
				data.indices.push_back(second);
				data.indices.push_back(first + 1);
			}

			if (stack != stackCount - 1)
			{
				data.indices.push_back(first + 1);
				data.indices.push_back(second);
				data.indices.push_back(second + 1);
			}
		}
	}

	return data;
}
