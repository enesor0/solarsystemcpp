#pragma once

#include <string>

class Texture
{
public:
	explicit Texture(const std::string& imagePath);
	~Texture();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	void bind(unsigned int textureUnit = 0) const;

private:
	unsigned int textureId_ = 0;
};
