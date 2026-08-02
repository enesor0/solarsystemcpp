#include "graphics/texture.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>

Texture::Texture(const std::string& imagePath)
{
	stbi_set_flip_vertically_on_load(true);

	int width = 0;
	int height = 0;
	int channelCount = 0;

	unsigned char* pixels = stbi_load(
		imagePath.c_str(),
		&width,
		&height,
		&channelCount,
		0
	);

	if (pixels == nullptr)
	{
		throw std::runtime_error(
			"Texture yuklenemedi: " + imagePath
		);
	}

	GLenum format = GL_RGB;

	if (channelCount == 1)
	{
		format = GL_RED;
	}
	else if (channelCount == 4)
	{
		format = GL_RGBA;
	}
	else if (channelCount != 3)
	{
		stbi_image_free(pixels);

		throw std::runtime_error(
			"Desteklenmeyen texture kanal sayisi: "
			+ std::to_string(channelCount)
		);
	}

	glGenTextures(1, &textureId_);
	glBindTexture(GL_TEXTURE_2D, textureId_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		static_cast<int>(format),
		width,
		height,
		0,
		format,
		GL_UNSIGNED_BYTE,
		pixels
	);
	glGenerateMipmap(GL_TEXTURE_2D);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	stbi_image_free(pixels);
}

Texture::~Texture()
{
	if (textureId_ != 0)
	{
		glDeleteTextures(1, &textureId_);
	}
}

void Texture::bind(unsigned int textureUnit) const
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, textureId_);
}
