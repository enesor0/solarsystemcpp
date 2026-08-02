#pragma once

#include "graphics/texture.h"

#include <memory>
#include <string>
#include <unordered_map>

class TextureLibrary
{
public:
	void load(const std::string& name, const std::string& imagePath);

	const Texture* find(const std::string& name) const;

private:
	std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
};
