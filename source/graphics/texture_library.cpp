#include "graphics/texture_library.h"

#include <stdexcept>

void TextureLibrary::load(
	const std::string& name,
	const std::string& imagePath)
{
	if (name.empty())
	{
		throw std::invalid_argument("Texture adi bos olamaz.");
	}

	if (textures_.find(name) != textures_.end())
	{
		throw std::invalid_argument(
			"Ayni ada sahip texture zaten yuklendi: " + name
		);
	}

	textures_.emplace(name, std::make_unique<Texture>(imagePath));
}

const Texture* TextureLibrary::find(const std::string& name) const
{
	const auto texture = textures_.find(name);

	if (texture == textures_.end())
	{
		return nullptr;
	}

	return texture->second.get();
}
