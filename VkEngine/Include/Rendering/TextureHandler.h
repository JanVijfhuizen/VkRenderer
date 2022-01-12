#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"

struct Texture final
{
	glm::ivec2 resolution;
	uint8_t channels;

	VkImage image;
	VkImageView imageView;
	VkDeviceMemory memory;
};

class TextureHandler final : public vi::VkHandler
{
public:
	explicit TextureHandler(vi::VkCore& core);

	[[nodiscard]] Texture Create(const char* name, const char* extension) const;
	void Destroy(const Texture& texture) const;

private:
	static unsigned char* Load(const std::string& path, int32_t& width, int32_t& height, int32_t& numChannels);
	static void Free(unsigned char* pixels);
};
