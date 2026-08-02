#pragma once

#include <glm/glm.hpp>

#include <array>

class ShadowMap
{
public:
	ShadowMap();
	~ShadowMap();

	ShadowMap(const ShadowMap&) = delete;
	ShadowMap& operator=(const ShadowMap&) = delete;

	std::array<glm::mat4, 6> lightSpaceMatrices(
		const glm::vec3& lightPosition
	) const;
	void beginRender(unsigned int face) const;
	void endRender(int framebufferWidth, int framebufferHeight) const;
	void bind(unsigned int textureUnit) const;

	float farPlane() const;

private:
	unsigned int framebuffer_ = 0;
	unsigned int depthCubemap_ = 0;

	static constexpr int resolution_ = 1024;
	static constexpr float nearPlane_ = 0.1f;
	static constexpr float farPlane_ = 30.0f;
};
