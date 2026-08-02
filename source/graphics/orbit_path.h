#pragma once

class OrbitPath
{
public:
	explicit OrbitPath(int segmentCount = 128);
	~OrbitPath();

	OrbitPath(const OrbitPath&) = delete;
	OrbitPath& operator=(const OrbitPath&) = delete;

	void draw() const;

private:
	unsigned int vao_ = 0;
	unsigned int vbo_ = 0;
	int vertexCount_ = 0;
};
