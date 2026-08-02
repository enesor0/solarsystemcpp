#include "graphics/orbit_path.h"

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <cmath>
#include <stdexcept>
#include <vector>

OrbitPath::OrbitPath(int segmentCount)
{
	if (segmentCount < 3)
	{
		throw std::invalid_argument(
			"Yorunge en az uc parcadan olusmalidir."
		);
	}

	vertexCount_ = segmentCount;

	std::vector<glm::vec3> vertices;
	vertices.reserve(static_cast<std::size_t>(segmentCount));

	constexpr float pi = 3.14159265359f;

	for (int segment = 0; segment < segmentCount; ++segment)
	{
		const float angle =
			2.0f * pi * static_cast<float>(segment)
			/ static_cast<float>(segmentCount);

		vertices.emplace_back(
			std::cos(angle),
			0.0f,
			std::sin(angle)
		);
	}

	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3)),
		vertices.data(),
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(glm::vec3),
		nullptr
	);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

OrbitPath::~OrbitPath()
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

void OrbitPath::draw() const
{
	glBindVertexArray(vao_);
	glDrawArrays(GL_LINE_LOOP, 0, vertexCount_);
	glBindVertexArray(0);
}
