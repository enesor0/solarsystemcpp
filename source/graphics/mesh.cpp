#include "graphics/mesh.h"

#include <glad/glad.h>

#include <cstddef>
#include <stdexcept>

Mesh::Mesh(const MeshData& meshData)
{
	if (meshData.vertices.empty() || meshData.indices.empty())
	{
		throw std::runtime_error(
			"Mesh icin vertex ve index verisi gereklidir."
		);
	}

	indexCount_ = static_cast<int>(meshData.indices.size());

	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);
	glGenBuffers(1, &ebo_);

	glBindVertexArray(vao_);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(meshData.vertices.size() * sizeof(Vertex)),
		meshData.vertices.data(),
		GL_STATIC_DRAW
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(
			meshData.indices.size() * sizeof(unsigned int)
			),
		meshData.indices.data(),
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		reinterpret_cast<const void*>(offsetof(Vertex, position))
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		reinterpret_cast<const void*>(offsetof(Vertex, normal))
	);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		2,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		reinterpret_cast<const void*>(offsetof(Vertex, textureCoords))
	);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

Mesh::~Mesh()
{
	if (ebo_ != 0)
	{
		glDeleteBuffers(1, &ebo_);
	}

	if (vbo_ != 0)
	{
		glDeleteBuffers(1, &vbo_);
	}

	if (vao_ != 0)
	{
		glDeleteVertexArrays(1, &vao_);
	}
}

void Mesh::draw() const
{
	glBindVertexArray(vao_);

	glDrawElements(
		GL_TRIANGLES,
		indexCount_,
		GL_UNSIGNED_INT,
		nullptr
	);

	glBindVertexArray(0);
}
