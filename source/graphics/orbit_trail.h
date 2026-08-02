#pragma once

#include <glm/glm.hpp>

#include <cstddef>
#include <vector>

class OrbitTrail
{
public:
	OrbitTrail();
	~OrbitTrail();

	OrbitTrail(const OrbitTrail&) = delete;
	OrbitTrail& operator=(const OrbitTrail&) = delete;

	void reset();
	void update(const glm::vec3& position, float time);
	void draw();

private:
	struct Sample
	{
		glm::vec3 position;
		float time = 0.0f;
	};

	void upload();

	unsigned int vertexArray_ = 0;
	unsigned int vertexBuffer_ = 0;
	std::vector<Sample> samples_;
	bool gpuDataNeedsUpdate_ = false;

	static constexpr std::size_t maximumSampleCount_ = 96;
	static constexpr float lifetimeSeconds_ = 6.0f;
	static constexpr float minimumSampleDistance_ = 0.025f;
};
