#pragma once

#include "graphics/shader.h"

class Bloom
{
public:
	Bloom(int width, int height, int blurPassCount = 8);
	~Bloom();

	Bloom(const Bloom&) = delete;
	Bloom& operator=(const Bloom&) = delete;

	void beginRender(int width, int height);
	void endRender(float exposure, float bloomStrength);

private:
	void createRenderTargets(int width, int height);
	void destroyRenderTargets();
	void renderScreenQuad() const;

	Shader brightnessShader_;
	Shader blurShader_;
	Shader compositeShader_;

	unsigned int sceneFramebuffer_ = 0;
	unsigned int sceneTexture_ = 0;
	unsigned int depthRenderbuffer_ = 0;

	unsigned int brightnessFramebuffer_ = 0;
	unsigned int brightnessTexture_ = 0;

	unsigned int blurFramebuffers_[2]{};
	unsigned int blurTextures_[2]{};

	unsigned int quadVao_ = 0;
	unsigned int quadVbo_ = 0;

	int width_ = 0;
	int height_ = 0;
	int blurPassCount_ = 0;
};
