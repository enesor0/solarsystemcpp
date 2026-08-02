#include "graphics/orbit_trail.h"

#include <glad/glad.h>

#include <glm/geometric.hpp>

#include <cstddef>

OrbitTrail::OrbitTrail()
{
	glGenVertexArrays(1, &vertexArray_);
	glGenBuffers(1, &vertexBuffer_);

	glBindVertexArray(vertexArray_);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		static_cast<GLsizei>(sizeof(Sample)),
		reinterpret_cast<void*>(offsetof(Sample, position))
	);
	glBindVertexArray(0);
}

OrbitTrail::~OrbitTrail()
{
	if (vertexBuffer_ != 0)
	{
		glDeleteBuffers(1, &vertexBuffer_);
	}

	if (vertexArray_ != 0)
	{
		glDeleteVertexArrays(1, &vertexArray_);
	}
}

void OrbitTrail::reset()
{
	if (samples_.empty())
	{
		return;
	}

	samples_.clear();
	gpuDataNeedsUpdate_ = true;
}

void OrbitTrail::update(const glm::vec3& position, float time)
{
	while (!samples_.empty()
		&& time - samples_.front().time > lifetimeSeconds_)
	{
		samples_.erase(samples_.begin());
		gpuDataNeedsUpdate_ = true;
	}

	const bool shouldAddSample = samples_.empty()
		|| glm::distance(samples_.back().position, position)
			>= minimumSampleDistance_;

	if (!shouldAddSample)
	{
		return;
	}

	samples_.push_back({ position, time });

	if (samples_.size() > maximumSampleCount_)
	{
		samples_.erase(samples_.begin());
	}

	gpuDataNeedsUpdate_ = true;
}

void OrbitTrail::draw()
{
	if (samples_.size() < 2)
	{
		return;
	}


	if (gpuDataNeedsUpdate_)
	{
		upload();
	}

	glBindVertexArray(vertexArray_);
	glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(samples_.size()));
	glBindVertexArray(0);
}

void OrbitTrail::upload()
{
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(samples_.size() * sizeof(Sample)),
		samples_.data(),
		GL_DYNAMIC_DRAW
	);

	gpuDataNeedsUpdate_ = false;
}
