#pragma once

#include <glm/glm.hpp>

#include <array>
#include <cstddef>
#include <string_view>
#include <vector>

class Shader;

struct HudFocusInfo
{
	std::string_view name;
	float radius = 0.0f;
	float orbitRadius = 0.0f;
	float orbitSpeed = 0.0f;
	std::size_t moonCount = 0;
	std::string_view summary;
	unsigned int diameterKilometers = 0;
	float averageDistanceMillionKm = 0.0f;
};

struct HudGraphicsInfo
{
	std::string_view qualityName;
	int shadowSampleCount = 0;
	std::size_t asteroidCount = 0;
	bool shadowsEnabled = false;
	bool atmospheresEnabled = false;
	bool cloudsEnabled = false;
	int resolutionWidth = 0;
	int resolutionHeight = 0;
	bool vSyncEnabled = true;
	float fieldOfViewDegrees = 50.0f;
	float interfaceScale = 1.0f;
	bool isSettingsPanelVisible = false;
};

class Hud
{
public:
	Hud();
	~Hud();

	Hud(const Hud&) = delete;
	Hud& operator=(const Hud&) = delete;

	void render(
		Shader& shader,
		int framebufferWidth,
		int framebufferHeight,
		const HudFocusInfo& focusInfo,
		bool isPaused,
		float timeScale,
		bool showOrbitPaths,
		const HudGraphicsInfo& graphicsInfo,
		float interfaceScale
	);

private:
	struct Vertex
	{
		glm::vec2 position;
		glm::vec4 color;
		glm::vec2 textureCoordinates;
		float textMask = 0.0f;
	};

	struct Glyph
	{
		float atlasLeft = 0.0f;
		float atlasTop = 0.0f;
		float atlasRight = 0.0f;
		float atlasBottom = 0.0f;
		float xOffset = 0.0f;
		float yOffset = 0.0f;
		float xAdvance = 0.0f;
	};

	void appendRectangle(
		const glm::vec2& position,
		const glm::vec2& size,
		const glm::vec4& color
	);
	void appendText(
		const glm::vec2& position,
		std::string_view text,
		const glm::vec4& color,
		float scale
	);
	void loadFont();

	unsigned int vertexArray_ = 0;
	unsigned int vertexBuffer_ = 0;
	unsigned int fontTexture_ = 0;
	std::array<Glyph, 95> glyphs_{};
	std::vector<Vertex> vertices_;

	static constexpr int fontAtlasSize_ = 512;
	static constexpr float fontBakeHeight_ = 36.0f;
};
