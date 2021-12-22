#pragma once
#include "UVector.h"

struct Texture final
{
	glm::ivec2 resolution;
	uint32_t channels;
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	class Manager final : public Singleton<Manager>
	{
	public:
		Manager();
		~Manager();

		[[nodiscard]] uint32_t CreateTexture(const std::string& fileName);
		[[nodiscard]] const Texture& GetData(uint32_t handle);

	private:
		UVector<Texture> _data;

		static unsigned char* Load(const std::string& path, int32_t& width, int32_t& height, int32_t& numChannels);
		static void Free(unsigned char* pixels);
	};
};
