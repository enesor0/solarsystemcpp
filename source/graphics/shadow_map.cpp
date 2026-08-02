#include "graphics/shadow_map.h"

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>

ShadowMap::ShadowMap()
{
	glGenFramebuffers(1, &framebuffer_);
	glGenTextures(1, &depthCubemap_);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap_);

	for (unsigned int face = 0; face < 6; ++face)
	{
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
			0,
			GL_DEPTH_COMPONENT24,
			resolution_,
			resolution_,
			0,
			GL_DEPTH_COMPONENT,
			GL_FLOAT,
			nullptr
		);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
	glFramebufferTexture(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		depthCubemap_,
		0
	);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		throw std::runtime_error("Golge framebuffer'i olusturulamadi.");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

ShadowMap::~ShadowMap()
{
	if (depthCubemap_ != 0)
	{
		glDeleteTextures(1, &depthCubemap_);
	}

	if (framebuffer_ != 0)
	{
		glDeleteFramebuffers(1, &framebuffer_);
	}
}

std::array<glm::mat4, 6> ShadowMap::lightSpaceMatrices(
	const glm::vec3& lightPosition) const
{
	const glm::mat4 projection = glm::perspective(
		glm::radians(90.0f),
		1.0f,
		nearPlane_,
		farPlane_
	);

	return {
		projection * glm::lookAt(
			lightPosition,
			lightPosition + glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f)
		),
		projection * glm::lookAt(
			lightPosition,
			lightPosition + glm::vec3(-1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f)
		),
		projection * glm::lookAt(
			lightPosition,
			lightPosition + glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		),
		projection * glm::lookAt(
			lightPosition,
			lightPosition + glm::vec3(0.0f, -1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, -1.0f)
		),
		projection * glm::lookAt(
			lightPosition,
			lightPosition + glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.0f, -1.0f, 0.0f)
		),
		projection * glm::lookAt(
			lightPosition,
			lightPosition + glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec3(0.0f, -1.0f, 0.0f)
		)
	};
}

void ShadowMap::beginRender(unsigned int face) const
{
	glViewport(0, 0, resolution_, resolution_);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
		depthCubemap_,
		0
	);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::endRender(int framebufferWidth, int framebufferHeight) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, framebufferWidth, framebufferHeight);
}

void ShadowMap::bind(unsigned int textureUnit) const
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap_);
}

float ShadowMap::farPlane() const
{
	return farPlane_;
}
