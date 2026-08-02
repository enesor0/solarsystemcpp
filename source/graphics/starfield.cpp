#include "graphics/starfield.h"

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <cmath>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
	struct StarVertex
	{
		glm::vec3 position;
		glm::vec3 color;
		float size;
	};
}

Starfield::Starfield(unsigned int starCount)
{
	if (starCount == 0)
	{
		throw std::invalid_argument("Yildiz sayisi sifir olamaz.");
	}

	starCount_ = static_cast<int>(starCount);

	std::mt19937 generator(1337);
	std::uniform_real_distribution<float> directionDistribution(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distanceDistribution(35.0f, 55.0f);
	std::uniform_real_distribution<float> brightnessDistribution(0.45f, 1.0f);
	std::uniform_real_distribution<float> sizeDistribution(1.0f, 3.0f);

	std::vector<StarVertex> stars;
	stars.reserve(starCount);

	while (stars.size() < starCount)
	{
		glm::vec3 direction(
			directionDistribution(generator),
			directionDistribution(generator),
			directionDistribution(generator)
		);

		const float length = glm::length(direction);

		if (length < 0.001f)
		{
			continue;
		}

		direction /= length;

		const float brightness = brightnessDistribution(generator);
		const glm::vec3 color(
			brightness,
			brightness * 0.96f,
			brightness * 0.92f
		);

		stars.push_back(StarVertex{
			direction * distanceDistribution(generator),
			color,
			sizeDistribution(generator)
		});
	}

	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(stars.size() * sizeof(StarVertex)),
		stars.data(),
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(StarVertex),
		reinterpret_cast<const void*>(offsetof(StarVertex, position))
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(StarVertex),
		reinterpret_cast<const void*>(offsetof(StarVertex, color))
	);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		2,
		1,
		GL_FLOAT,
		GL_FALSE,
		sizeof(StarVertex),
		reinterpret_cast<const void*>(offsetof(StarVertex, size))
	);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

Starfield::~Starfield()
{
	if (vbo_ != 0)
	{
		glDeleteBuffers(1, &vbo_);
	}

	if (vao_ != 0)
	{
		glDeleteVertexArrays(1, &vao_);
	}
}

void Starfield::draw() const
{
	glBindVertexArray(vao_);
	glDrawArrays(GL_POINTS, 0, starCount_);
	glBindVertexArray(0);
}
