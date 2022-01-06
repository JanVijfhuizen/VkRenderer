#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkImageHandler final : public VkHandler
	{
		friend VkCore;

	public:
		[[nodiscard]] VkImageView CreateView(VkImage image, 
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) const;
		void DestroyView(VkImageView imageView) const;

	private:
		explicit VkImageHandler(VkCore& core);
	};
}
