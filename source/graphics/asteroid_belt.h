#pragma once

#include <cstddef>

class AsteroidBelt
{
public:
	explicit AsteroidBelt(std::size_t asteroidCount);
	~AsteroidBelt();

	AsteroidBelt(const AsteroidBelt&) = delete;
	AsteroidBelt& operator=(const AsteroidBelt&) = delete;

	void draw(std::size_t visibleAsteroidCount) const;

private:
	unsigned int vertexArray_ = 0;
	unsigned int meshVertexBuffer_ = 0;
	unsigned int indexBuffer_ = 0;
	unsigned int instanceBuffer_ = 0;
	int indexCount_ = 0;
	int asteroidCount_ = 0;
};
