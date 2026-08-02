#include "graphics/bloom.h"

#include <glad/glad.h>

#include <stdexcept>
#include <string>

namespace
{
	void checkFramebuffer(const char* targetName)
	{
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			throw std::runtime_error(
				std::string("Bloom framebuffer hatasi: ") + targetName
			);
		}
	}

	void configureColorTexture(unsigned int texture, int width, int height)
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGB16F,
			width,
			height,
			0,
			GL_RGB,
			GL_FLOAT,
			nullptr
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
}

Bloom::Bloom(int width, int height, int blurPassCount)
	: brightnessShader_(
		"shaders/post_process.vert",
		"shaders/bloom_extract.frag"
	)
	, blurShader_(
		"shaders/post_process.vert",
		"shaders/bloom_blur.frag"
	)
	, compositeShader_(
		"shaders/post_process.vert",
		"shaders/bloom_composite.frag"
	)
	, blurPassCount_(blurPassCount)
{
	if (width <= 0 || height <= 0 || blurPassCount <= 0)
	{
		throw std::invalid_argument(
			"Bloom boyutlari ve blur gecis sayisi pozitif olmalidir."
		);
	}

	const float screenQuadVertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f,  1.0f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &quadVao_);
	glGenBuffers(1, &quadVbo_);

	glBindVertexArray(quadVao_);
	glBindBuffer(GL_ARRAY_BUFFER, quadVbo_);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(screenQuadVertices),
		screenQuadVertices,
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		4 * sizeof(float),
		reinterpret_cast<const void*>(2 * sizeof(float))
	);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	createRenderTargets(width, height);
}

Bloom::~Bloom()
{
	destroyRenderTargets();

	if (quadVbo_ != 0)
	{
		glDeleteBuffers(1, &quadVbo_);
	}

	if (quadVao_ != 0)
	{
		glDeleteVertexArrays(1, &quadVao_);
	}
}

void Bloom::beginRender(int width, int height)
{
	if (width != width_ || height != height_)
	{
		createRenderTargets(width, height);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, sceneFramebuffer_);
	glViewport(0, 0, width_, height_);
}

void Bloom::endRender(float exposure, float bloomStrength)
{
	glDisable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, brightnessFramebuffer_);
	glClear(GL_COLOR_BUFFER_BIT);

	brightnessShader_.use();
	brightnessShader_.setInt("sceneTexture", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneTexture_);
	renderScreenQuad();

	bool horizontal = true;
	bool firstPass = true;

	for (int pass = 0; pass < blurPassCount_; ++pass)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, blurFramebuffers_[horizontal]);
		glClear(GL_COLOR_BUFFER_BIT);

		blurShader_.use();
		blurShader_.setInt("sourceTexture", 0);
		blurShader_.setInt("horizontal", horizontal ? 1 : 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(
			GL_TEXTURE_2D,
			firstPass ? brightnessTexture_ : blurTextures_[!horizontal]
		);

		renderScreenQuad();

		horizontal = !horizontal;
		firstPass = false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width_, height_);
	glClear(GL_COLOR_BUFFER_BIT);

	compositeShader_.use();
	compositeShader_.setInt("sceneTexture", 0);
	compositeShader_.setInt("bloomTexture", 1);
	compositeShader_.setFloat("exposure", exposure);
	compositeShader_.setFloat("bloomStrength", bloomStrength);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneTexture_);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, blurTextures_[!horizontal]);

	renderScreenQuad();

	glEnable(GL_DEPTH_TEST);
}

void Bloom::createRenderTargets(int width, int height)
{
	if (width <= 0 || height <= 0)
	{
		return;
	}

	destroyRenderTargets();

	width_ = width;
	height_ = height;

	glGenFramebuffers(1, &sceneFramebuffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFramebuffer_);

	glGenTextures(1, &sceneTexture_);
	configureColorTexture(sceneTexture_, width_, height_);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		sceneTexture_,
		0
	);

	glGenRenderbuffers(1, &depthRenderbuffer_);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer_);
	glRenderbufferStorage(
		GL_RENDERBUFFER,
		GL_DEPTH_COMPONENT24,
		width_,
		height_
	);
	glFramebufferRenderbuffer(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER,
		depthRenderbuffer_
	);

	checkFramebuffer("scene");

	glGenFramebuffers(1, &brightnessFramebuffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, brightnessFramebuffer_);

	glGenTextures(1, &brightnessTexture_);
	configureColorTexture(brightnessTexture_, width_, height_);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		brightnessTexture_,
		0
	);

	checkFramebuffer("brightness");

	glGenFramebuffers(2, blurFramebuffers_);
	glGenTextures(2, blurTextures_);

	for (int target = 0; target < 2; ++target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, blurFramebuffers_[target]);
		configureColorTexture(blurTextures_[target], width_, height_);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D,
			blurTextures_[target],
			0
		);

		checkFramebuffer("blur");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Bloom::destroyRenderTargets()
{
	if (depthRenderbuffer_ != 0)
	{
		glDeleteRenderbuffers(1, &depthRenderbuffer_);
		depthRenderbuffer_ = 0;
	}

	if (sceneTexture_ != 0)
	{
		glDeleteTextures(1, &sceneTexture_);
		sceneTexture_ = 0;
	}

	if (brightnessTexture_ != 0)
	{
		glDeleteTextures(1, &brightnessTexture_);
		brightnessTexture_ = 0;
	}

	glDeleteTextures(2, blurTextures_);
	blurTextures_[0] = 0;
	blurTextures_[1] = 0;

	if (sceneFramebuffer_ != 0)
	{
		glDeleteFramebuffers(1, &sceneFramebuffer_);
		sceneFramebuffer_ = 0;
	}

	if (brightnessFramebuffer_ != 0)
	{
		glDeleteFramebuffers(1, &brightnessFramebuffer_);
		brightnessFramebuffer_ = 0;
	}

	glDeleteFramebuffers(2, blurFramebuffers_);
	blurFramebuffers_[0] = 0;
	blurFramebuffers_[1] = 0;
}

void Bloom::renderScreenQuad() const
{
	glBindVertexArray(quadVao_);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
