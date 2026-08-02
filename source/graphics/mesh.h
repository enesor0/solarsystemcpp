#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 textureCoords;
};

struct MeshData
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

class Mesh
{
public:
	explicit Mesh(const MeshData& meshData);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	void draw() const;

private:
	unsigned int vao_ = 0;
	unsigned int vbo_ = 0;
	unsigned int ebo_ = 0;
	int indexCount_ = 0;
};
