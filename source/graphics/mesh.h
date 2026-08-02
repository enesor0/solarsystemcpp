#pragma once

#include <vector>

class mesh
{
public:
	explicit mesh(const std::vector<float>& positions);
	~mesh();

	mesh(const mesh&) = delete;
	mesh& operator=(const mesh&) = delete;

	void draw() const;


private:
	unsigned int vao_ = 0;
	unsigned int vbo_ = 0;
	int vertexCount_ = 0;
};