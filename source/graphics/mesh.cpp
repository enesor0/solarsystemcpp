#include "graphics/Mesh.h"

#include <glad/glad.h>

#include <stdexcept>

mesh::mesh(const std::vector<float>& positions)
{
	if (positions.empty() || positions.size() % 3 != 0)
	{
		throw std::runtime_error(
			"Mesh konum verisi 3'erli koordinatlardan olusmalidir."
		);
	}

	vertexCount_ = static_cast<int>(positions.size() / 3);

	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);

	glBindVertexArray(vao_);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(positions.size() * sizeof(float)),
		positions.data(),
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		3 * sizeof(float),
		nullptr
	);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

mesh::~mesh()
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

void mesh::draw() const
{
	glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
	glBindVertexArray(0);
}