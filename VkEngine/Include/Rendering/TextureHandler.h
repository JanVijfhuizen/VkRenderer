#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"

/// <summary>
/// Contains all relevant texture information.
/// </summary>
struct Texture final
{
	glm::ivec2 resolution;
	uint8_t channels;
	uint32_t mipLevels;

	VkImage image;
	VkImageView imageView;
	VkDeviceMemory memory;
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
};

/// <summary>
/// Contains some core functionality for textures.
/// </summary>
class TextureHandler final : public vi::VkHandler
{
public:
	explicit TextureHandler(vi::VkCore& core);

	// Creates a new texture. Assumes the texture is in the correct folder.
	[[nodiscard]] Texture Create(const char* name, const char* extension) const;
	// Destroys the resources for target texture.
	void Destroy(const Texture& texture) const;

private:
	// Generate mip maps for an image.
	void GenerateMipMaps(VkImage image, glm::ivec2 resolution, uint32_t mipLevels, VkFormat imageFormat) const;

	static unsigned char* Load(const std::string& path, int32_t& width, int32_t& height, int32_t& numChannels);
	static void Free(unsigned char* pixels);
};
