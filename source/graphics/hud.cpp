#include "graphics/hud.h"

#include "graphics/shader.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	std::vector<unsigned char> readBinaryFile(const char* path)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);

		if (!file)
		{
			return {};
		}

		const std::streamsize fileSize = file.tellg();

		if (fileSize <= 0)
		{
			return {};
		}

		std::vector<unsigned char> bytes(static_cast<std::size_t>(fileSize));
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(bytes.data()), fileSize);

		return file ? bytes : std::vector<unsigned char>{};
	}
}

Hud::Hud()
{
	glGenVertexArrays(1, &vertexArray_);
	glGenBuffers(1, &vertexBuffer_);

	glBindVertexArray(vertexArray_);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		static_cast<GLsizei>(sizeof(Vertex)),
		reinterpret_cast<void*>(offsetof(Vertex, position))
	);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		4,
		GL_FLOAT,
		GL_FALSE,
		static_cast<GLsizei>(sizeof(Vertex)),
		reinterpret_cast<void*>(offsetof(Vertex, color))
	);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,
		2,
		GL_FLOAT,
		GL_FALSE,
		static_cast<GLsizei>(sizeof(Vertex)),
		reinterpret_cast<void*>(offsetof(Vertex, textureCoordinates))
	);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		3,
		1,
		GL_FLOAT,
		GL_FALSE,
		static_cast<GLsizei>(sizeof(Vertex)),
		reinterpret_cast<void*>(offsetof(Vertex, textMask))
	);

	glBindVertexArray(0);

	loadFont();
}

Hud::~Hud()
{
	if (fontTexture_ != 0)
	{
		glDeleteTextures(1, &fontTexture_);
	}

	if (vertexBuffer_ != 0)
	{
		glDeleteBuffers(1, &vertexBuffer_);
	}

	if (vertexArray_ != 0)
	{
		glDeleteVertexArrays(1, &vertexArray_);
	}
}

void Hud::render(
	Shader& shader,
	int framebufferWidth,
	int framebufferHeight,
	const HudFocusInfo& focusInfo,
	bool isPaused,
	float timeScale,
	bool showOrbitPaths,
	const HudGraphicsInfo& graphicsInfo,
	float interfaceScale)
{
	if (framebufferWidth <= 0 || framebufferHeight <= 0)
	{
		return;
	}

	vertices_.clear();

	const float resolutionScale = std::clamp(
		std::min(
			static_cast<float>(framebufferWidth) / 1920.0f,
			static_cast<float>(framebufferHeight) / 1080.0f
		),
		0.85f,
		1.40f
	);
	const float scale = std::clamp(
		resolutionScale * interfaceScale,
		0.72f,
		1.55f
	);
	const float margin = std::max(
		24.0f * scale,
		static_cast<float>(framebufferWidth) * 0.022f
	);
	const bool hasFocusedPlanet = !focusInfo.name.empty();
	const glm::vec2 panelPosition(margin, margin);
	const glm::vec2 panelSize(
		330.0f * scale,
		(hasFocusedPlanet ? 190.0f : 148.0f) * scale
	);
	const glm::vec4 panelColor(0.004f, 0.012f, 0.030f, 0.72f);
	const glm::vec4 headerColor(0.012f, 0.046f, 0.095f, 0.78f);
	const glm::vec4 accentColor(0.30f, 0.82f, 1.00f, 0.94f);
	const glm::vec4 primaryText(0.90f, 0.96f, 1.00f, 0.96f);
	const glm::vec4 mutedText(0.52f, 0.68f, 0.82f, 0.90f);
	const glm::vec4 pausedText(1.00f, 0.71f, 0.32f, 0.98f);

	appendRectangle(panelPosition, panelSize, panelColor);
	appendRectangle(
		panelPosition,
		glm::vec2(2.0f * scale, panelSize.y),
		accentColor
	);
	appendRectangle(
		panelPosition + glm::vec2(2.0f * scale, 0.0f),
		glm::vec2(panelSize.x - 2.0f * scale, 36.0f * scale),
		headerColor
	);
	appendRectangle(
		panelPosition + glm::vec2(15.0f * scale, 36.0f * scale),
		glm::vec2(panelSize.x - 30.0f * scale, 1.0f * scale),
		glm::vec4(accentColor.r, accentColor.g, accentColor.b, 0.34f)
	);

	const float titleScale = 0.66f * scale;
	const float bodyScale = 0.55f * scale;
	const float hintScale = 0.44f * scale;
	const float textX = panelPosition.x + 16.0f * scale;
	const float headerY = panelPosition.y + 5.0f * scale;

	appendText(
		glm::vec2(textX, headerY),
		"SOLAR SYSTEM // LIVE",
		accentColor,
		titleScale
	);

	const std::string target = focusInfo.name.empty()
		? "FREE CAMERA"
		: std::string(focusInfo.name);
	float lineY = panelPosition.y + 43.0f * scale;

	appendText(
		glm::vec2(textX, lineY),
		"TARGET  " + target,
		primaryText,
		bodyScale
	);
	lineY += 20.0f * scale;

	if (hasFocusedPlanet)
	{
		appendText(
			glm::vec2(textX, lineY),
			std::string(focusInfo.summary),
			accentColor,
			bodyScale
		);
		lineY += 20.0f * scale;

		std::ostringstream physicalInfo;
		physicalInfo << "DIA " << focusInfo.diameterKilometers
			<< " KM  /  DIST " << std::fixed << std::setprecision(1)
			<< focusInfo.averageDistanceMillionKm << " M KM";

		appendText(
			glm::vec2(textX, lineY),
			physicalInfo.str(),
			mutedText,
			bodyScale
		);
		lineY += 20.0f * scale;

		std::ostringstream satelliteInfo;
		satelliteInfo << "MOONS " << focusInfo.moonCount
			<< "  /  ORBIT " << std::fixed << std::setprecision(2)
			<< focusInfo.orbitSpeed;

		appendText(
			glm::vec2(textX, lineY),
			satelliteInfo.str(),
			mutedText,
			bodyScale
		);
		lineY += 23.0f * scale;
	}
	else
	{
		appendText(
			glm::vec2(textX, lineY),
			"PRESS 1-9 TO FOCUS",
			mutedText,
			bodyScale
		);
		lineY += 25.0f * scale;
	}

	std::ostringstream simulationText;
	simulationText << "SIM  "
		<< (isPaused ? "PAUSED" : "ACTIVE")
		<< "  x" << std::fixed << std::setprecision(2) << timeScale;

	appendText(
		glm::vec2(textX, lineY),
		simulationText.str(),
		isPaused ? pausedText : primaryText,
		bodyScale
	);
	lineY += 20.0f * scale;

	appendText(
		glm::vec2(textX, lineY),
		std::string("ORBIT PATHS  ") + (showOrbitPaths ? "ON" : "OFF"),
		mutedText,
		bodyScale
	);
	lineY += 23.0f * scale;

	appendText(
		glm::vec2(textX, lineY),
		"WASD MOVE  /  MOUSE LOOK  /  H HIDE",
		mutedText,
		hintScale
	);

	if (graphicsInfo.isSettingsPanelVisible)
	{
		const glm::vec2 settingsSize(368.0f * scale, 358.0f * scale);
		const glm::vec2 settingsPosition(
			static_cast<float>(framebufferWidth) - margin - settingsSize.x,
			margin
		);
		const glm::vec4 settingsPanelColor(0.014f, 0.026f, 0.060f, 0.88f);
		const glm::vec4 settingsHeaderColor(0.070f, 0.125f, 0.220f, 0.94f);
		const float settingsTextX = settingsPosition.x + 20.0f * scale;
		float settingsLineY = settingsPosition.y + 56.0f * scale;

		appendRectangle(settingsPosition, settingsSize, settingsPanelColor);
		appendRectangle(
			settingsPosition,
			glm::vec2(settingsSize.x, 42.0f * scale),
			settingsHeaderColor
		);
		appendRectangle(
			settingsPosition + glm::vec2(0.0f, 42.0f * scale),
			glm::vec2(settingsSize.x, 2.0f * scale),
			accentColor
		);
		appendText(
			glm::vec2(settingsTextX, settingsPosition.y + 7.0f * scale),
			"GRAPHICS SETTINGS",
			accentColor,
			titleScale
		);
		appendText(
			glm::vec2(settingsTextX, settingsLineY),
			"PROFILE  //  " + std::string(graphicsInfo.qualityName),
			primaryText,
			bodyScale
		);
		settingsLineY += 27.0f * scale;
		appendText(
			glm::vec2(settingsTextX, settingsLineY),
			"SHADOWS  " + std::string(graphicsInfo.shadowsEnabled ? "ON" : "OFF")
				+ "   PCF " + std::to_string(graphicsInfo.shadowSampleCount),
			mutedText,
			bodyScale
		);
		settingsLineY += 25.0f * scale;
		appendText(
			glm::vec2(settingsTextX, settingsLineY),
			"ATMOSPHERES  " + std::string(graphicsInfo.atmospheresEnabled ? "ON" : "OFF"),
			mutedText,
			bodyScale
		);
		settingsLineY += 25.0f * scale;
		appendText(
			glm::vec2(settingsTextX, settingsLineY),
			"CLOUDS  " + std::string(graphicsInfo.cloudsEnabled ? "ON" : "OFF"),
			mutedText,
			bodyScale
		);
		settingsLineY += 25.0f * scale;
		appendText(
			glm::vec2(settingsTextX, settingsLineY),
			"ASTEROIDS  " + std::to_string(graphicsInfo.asteroidCount),
			mutedText,
			bodyScale
		);
		settingsLineY += 25.0f * scale;
		appendText(
			glm::vec2(settingsTextX, settingsLineY),
			"RESOLUTION  " + std::to_string(graphicsInfo.resolutionWidth)
				+ " X " + std::to_string(graphicsInfo.resolutionHeight),
			mutedText,
			bodyScale
		);
		settingsLineY += 25.0f * scale;
		appendText(
			glm::vec2(settingsTextX, settingsLineY),
			"V-SYNC  " + std::string(graphicsInfo.vSyncEnabled ? "ON" : "OFF"),
			mutedText,
			bodyScale
		);
		settingsLineY += 25.0f * scale;
		appendText(
			glm::vec2(settingsTextX, settingsLineY),
			"FOV  " + std::to_string(
				static_cast<int>(graphicsInfo.fieldOfViewDegrees)
			) + " DEG",
			mutedText,
			bodyScale
		);
		settingsLineY += 25.0f * scale;
		appendText(
			glm::vec2(settingsTextX, settingsLineY),
			"UI SCALE  " + std::to_string(
				static_cast<int>(graphicsInfo.interfaceScale * 100.0f)
			) + "%",
			mutedText,
			bodyScale
		);
		settingsLineY += 31.0f * scale;
		appendText(
			glm::vec2(settingsTextX, settingsLineY),
			"G PROFILE   R RESOLUTION   V V-SYNC\n[ ] FOV   - = UI SCALE   F3 CLOSE",
			accentColor,
			hintScale
		);
	}

	shader.use();
	shader.setMat4(
		"projection",
		glm::ortho(
			0.0f,
			static_cast<float>(framebufferWidth),
			static_cast<float>(framebufferHeight),
			0.0f
		)
	);
	shader.setInt("fontTexture", 0);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fontTexture_);
	glBindVertexArray(vertexArray_);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(vertices_.size() * sizeof(Vertex)),
		vertices_.data(),
		GL_DYNAMIC_DRAW
	);
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_.size()));
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

void Hud::appendRectangle(
	const glm::vec2& position,
	const glm::vec2& size,
	const glm::vec4& color)
{
	const glm::vec2 topRight = position + glm::vec2(size.x, 0.0f);
	const glm::vec2 bottomLeft = position + glm::vec2(0.0f, size.y);
	const glm::vec2 bottomRight = position + size;

	vertices_.insert(
		vertices_.end(),
		{
			{ position, color, glm::vec2(0.0f), 0.0f },
			{ bottomLeft, color, glm::vec2(0.0f), 0.0f },
			{ topRight, color, glm::vec2(0.0f), 0.0f },
			{ topRight, color, glm::vec2(0.0f), 0.0f },
			{ bottomLeft, color, glm::vec2(0.0f), 0.0f },
			{ bottomRight, color, glm::vec2(0.0f), 0.0f }
		}
	);
}

void Hud::appendText(
	const glm::vec2& position,
	std::string_view text,
	const glm::vec4& color,
	float scale)
{
	float penX = position.x / scale;
	float penY = position.y / scale + fontBakeHeight_;
	const float lineStartX = penX;

	for (const unsigned char character : text)
	{
		if (character == '\n')
		{
			penX = lineStartX;
			penY += fontBakeHeight_;
			continue;
		}

		if (character < 32 || character > 126)
		{
			continue;
		}

		const Glyph& glyph = glyphs_.at(character - 32);
		const glm::vec2 topLeft(
			(penX + glyph.xOffset) * scale,
			(penY + glyph.yOffset) * scale
		);
		const glm::vec2 bottomRight = topLeft + glm::vec2(
			(glyph.atlasRight - glyph.atlasLeft) * scale,
			(glyph.atlasBottom - glyph.atlasTop) * scale
		);
		const glm::vec2 topRight(bottomRight.x, topLeft.y);
		const glm::vec2 bottomLeft(topLeft.x, bottomRight.y);
		const glm::vec2 textureTopLeft(
			glyph.atlasLeft / fontAtlasSize_,
			glyph.atlasTop / fontAtlasSize_
		);
		const glm::vec2 textureBottomRight(
			glyph.atlasRight / fontAtlasSize_,
			glyph.atlasBottom / fontAtlasSize_
		);
		const glm::vec2 textureTopRight(textureBottomRight.x, textureTopLeft.y);
		const glm::vec2 textureBottomLeft(textureTopLeft.x, textureBottomRight.y);

		vertices_.insert(
			vertices_.end(),
			{
				{ topLeft, color, textureTopLeft, 1.0f },
				{ bottomLeft, color, textureBottomLeft, 1.0f },
				{ topRight, color, textureTopRight, 1.0f },
				{ topRight, color, textureTopRight, 1.0f },
				{ bottomLeft, color, textureBottomLeft, 1.0f },
				{ bottomRight, color, textureBottomRight, 1.0f }
			}
		);

		penX += glyph.xAdvance;
	}
}

void Hud::loadFont()
{
	constexpr std::array<const char*, 3> fontPaths{
		"C:/Windows/Fonts/segoeui.ttf",
		"C:/Windows/Fonts/arial.ttf",
		"C:/Windows/Fonts/consola.ttf"
	};

	std::vector<unsigned char> fontData;

	for (const char* path : fontPaths)
	{
		fontData = readBinaryFile(path);

		if (!fontData.empty())
		{
			break;
		}
	}

	if (fontData.empty())
	{
		throw std::runtime_error("HUD icin Windows yazı tipi bulunamadi.");
	}

	std::array<stbtt_bakedchar, 95> bakedGlyphs{};
	std::vector<unsigned char> atlas(fontAtlasSize_ * fontAtlasSize_);
	const int bakedHeight = stbtt_BakeFontBitmap(
		fontData.data(),
		0,
		fontBakeHeight_,
		atlas.data(),
		fontAtlasSize_,
		fontAtlasSize_,
		32,
		static_cast<int>(bakedGlyphs.size()),
		bakedGlyphs.data()
	);

	if (bakedHeight <= 0)
	{
		throw std::runtime_error("HUD yazı tipi atlasi olusturulamadi.");
	}

	for (std::size_t index = 0; index < glyphs_.size(); ++index)
	{
		const stbtt_bakedchar& source = bakedGlyphs[index];
		glyphs_[index] = {
			static_cast<float>(source.x0),
			static_cast<float>(source.y0),
			static_cast<float>(source.x1),
			static_cast<float>(source.y1),
			source.xoff,
			source.yoff,
			source.xadvance
		};
	}

	glGenTextures(1, &fontTexture_);
	glBindTexture(GL_TEXTURE_2D, fontTexture_);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_R8,
		fontAtlasSize_,
		fontAtlasSize_,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		atlas.data()
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, 0);
}
