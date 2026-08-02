#include "graphics/asteroid_belt.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <random>
#include <vector>

namespace
{
	struct AsteroidVertex
	{
		glm::vec3 position;
	};

	struct AsteroidInstance
	{
		glm::mat4 localModel;
		glm::vec4 orbitData;
		glm::vec4 spinData;
		glm::vec4 orbitOrientation;
		glm::vec3 color;
	};

	std::array<AsteroidVertex, 12> makeIcosahedronVertices()
	{
		constexpr float goldenRatio = 1.61803398875f;
		std::array<AsteroidVertex, 12> vertices{
			AsteroidVertex{ glm::vec3(-1.0f, goldenRatio, 0.0f) },
			AsteroidVertex{ glm::vec3(1.0f, goldenRatio, 0.0f) },
			AsteroidVertex{ glm::vec3(-1.0f, -goldenRatio, 0.0f) },
			AsteroidVertex{ glm::vec3(1.0f, -goldenRatio, 0.0f) },
			AsteroidVertex{ glm::vec3(0.0f, -1.0f, goldenRatio) },
			AsteroidVertex{ glm::vec3(0.0f, 1.0f, goldenRatio) },
			AsteroidVertex{ glm::vec3(0.0f, -1.0f, -goldenRatio) },
			AsteroidVertex{ glm::vec3(0.0f, 1.0f, -goldenRatio) },
			AsteroidVertex{ glm::vec3(goldenRatio, 0.0f, -1.0f) },
			AsteroidVertex{ glm::vec3(goldenRatio, 0.0f, 1.0f) },
			AsteroidVertex{ glm::vec3(-goldenRatio, 0.0f, -1.0f) },
			AsteroidVertex{ glm::vec3(-goldenRatio, 0.0f, 1.0f) }
		};

		for (AsteroidVertex& vertex : vertices)
		{
			vertex.position = glm::normalize(vertex.position);
		}

		return vertices;
	}
}

AsteroidBelt::AsteroidBelt(std::size_t asteroidCount)
	: asteroidCount_(static_cast<int>(asteroidCount))
{
	const std::array<AsteroidVertex, 12> vertices = makeIcosahedronVertices();
	const std::array<unsigned int, 60> indices{
		0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
		1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 7, 1, 8,
		3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
		4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1
	};
	indexCount_ = static_cast<int>(indices.size());

	std::mt19937 randomEngine(7319);
	std::uniform_real_distribution<float> angleDistribution(0.0f, 6.2831853f);
	std::uniform_real_distribution<float> semiMajorAxisDistribution(2.1f, 3.3f);
	std::uniform_real_distribution<float> eccentricityDistribution(0.0f, 0.20f);
	std::uniform_real_distribution<float> inclinationDistribution(0.0f, 20.0f);
	std::uniform_real_distribution<float> longitudeDistribution(0.0f, 360.0f);
	std::uniform_real_distribution<float> scaleDistribution(0.012f, 0.045f);
	std::uniform_real_distribution<float> stretchDistribution(0.72f, 1.36f);
	std::uniform_real_distribution<float> rotationDistribution(0.0f, 6.2831853f);
	std::uniform_real_distribution<float> spinSpeedDistribution(0.25f, 1.20f);
	std::uniform_real_distribution<float> toneDistribution(0.34f, 0.72f);

	std::vector<AsteroidInstance> instances;
	instances.reserve(asteroidCount);

	for (std::size_t index = 0; index < asteroidCount; ++index)
	{
		const float angle = angleDistribution(randomEngine);
		const float physicalSemiMajorAxis = semiMajorAxisDistribution(randomEngine);
		// Map 2.1--3.3 AU to the compressed scene scale while keeping the
		// original readable visual cadence of the asteroid belt.
		const float radius = 5.45f + (physicalSemiMajorAxis - 2.1f)
			* (6.60f - 5.45f) / (3.3f - 2.1f);
		const float orbitSpeed = 0.08f + (
			physicalSemiMajorAxis - 2.1f
		) * (0.22f - 0.08f) / (3.3f - 2.1f);
		const float scale = scaleDistribution(randomEngine);
		const float tone = toneDistribution(randomEngine);
		const glm::vec3 rotationAxis = glm::normalize(glm::vec3(
			stretchDistribution(randomEngine),
			stretchDistribution(randomEngine),
			stretchDistribution(randomEngine)
		));

		glm::mat4 localModel(1.0f);
		localModel = glm::rotate(
			localModel,
			rotationDistribution(randomEngine),
			rotationAxis
		);
		localModel = glm::scale(
			localModel,
			glm::vec3(
				scale * stretchDistribution(randomEngine),
				scale * stretchDistribution(randomEngine),
				scale * stretchDistribution(randomEngine)
			)
		);

		instances.push_back({
			localModel,
			glm::vec4(
				radius,
				angle,
				orbitSpeed,
				eccentricityDistribution(randomEngine)
			),
			glm::vec4(rotationAxis, spinSpeedDistribution(randomEngine)),
			glm::vec4(
				inclinationDistribution(randomEngine),
				longitudeDistribution(randomEngine),
				longitudeDistribution(randomEngine),
				0.0f
			),
			glm::vec3(tone, tone * 0.82f, tone * 0.62f)
		});
	}

	glGenVertexArrays(1, &vertexArray_);
	glGenBuffers(1, &meshVertexBuffer_);
	glGenBuffers(1, &indexBuffer_);
	glGenBuffers(1, &instanceBuffer_);

	glBindVertexArray(vertexArray_);
	glBindBuffer(GL_ARRAY_BUFFER, meshVertexBuffer_);
	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(vertices.size() * sizeof(AsteroidVertex)),
		vertices.data(),
		GL_STATIC_DRAW
	);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		static_cast<GLsizei>(sizeof(AsteroidVertex)),
		reinterpret_cast<void*>(offsetof(AsteroidVertex, position))
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
		indices.data(),
		GL_STATIC_DRAW
	);

	glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer_);
	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(instances.size() * sizeof(AsteroidInstance)),
		instances.data(),
		GL_STATIC_DRAW
	);

	for (unsigned int column = 0; column < 4; ++column)
	{
		const unsigned int location = 1 + column;
		glEnableVertexAttribArray(location);
		glVertexAttribPointer(
			location,
			4,
			GL_FLOAT,
			GL_FALSE,
			static_cast<GLsizei>(sizeof(AsteroidInstance)),
			reinterpret_cast<void*>(
				offsetof(AsteroidInstance, localModel) + sizeof(glm::vec4) * column
			)
		);
		glVertexAttribDivisor(location, 1);
	}

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(
		5,
		4,
		GL_FLOAT,
		GL_FALSE,
		static_cast<GLsizei>(sizeof(AsteroidInstance)),
		reinterpret_cast<void*>(offsetof(AsteroidInstance, orbitData))
	);
	glVertexAttribDivisor(5, 1);

	glEnableVertexAttribArray(6);
	glVertexAttribPointer(
		6,
		4,
		GL_FLOAT,
		GL_FALSE,
		static_cast<GLsizei>(sizeof(AsteroidInstance)),
		reinterpret_cast<void*>(offsetof(AsteroidInstance, spinData))
	);
	glVertexAttribDivisor(6, 1);

	glEnableVertexAttribArray(7);
	glVertexAttribPointer(
		7,
		4,
		GL_FLOAT,
		GL_FALSE,
		static_cast<GLsizei>(sizeof(AsteroidInstance)),
		reinterpret_cast<void*>(offsetof(AsteroidInstance, orbitOrientation))
	);
	glVertexAttribDivisor(7, 1);

	glEnableVertexAttribArray(8);
	glVertexAttribPointer(
		8,
		3,
		GL_FLOAT,
		GL_FALSE,
		static_cast<GLsizei>(sizeof(AsteroidInstance)),
		reinterpret_cast<void*>(offsetof(AsteroidInstance, color))
	);
	glVertexAttribDivisor(8, 1);

	glBindVertexArray(0);
}

AsteroidBelt::~AsteroidBelt()
{
	if (instanceBuffer_ != 0)
	{
		glDeleteBuffers(1, &instanceBuffer_);
	}

	if (indexBuffer_ != 0)
	{
		glDeleteBuffers(1, &indexBuffer_);
	}

	if (meshVertexBuffer_ != 0)
	{
		glDeleteBuffers(1, &meshVertexBuffer_);
	}

	if (vertexArray_ != 0)
	{
		glDeleteVertexArrays(1, &vertexArray_);
	}
}

void AsteroidBelt::draw(std::size_t visibleAsteroidCount) const
{
	const int drawCount = std::min(
		asteroidCount_,
		static_cast<int>(visibleAsteroidCount)
	);

	glBindVertexArray(vertexArray_);
	glDrawElementsInstanced(
		GL_TRIANGLES,
		indexCount_,
		GL_UNSIGNED_INT,
		nullptr,
		drawCount
	);
	glBindVertexArray(0);
}
