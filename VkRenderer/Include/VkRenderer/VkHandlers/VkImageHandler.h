#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkImageHandler final : public VkHandler
	{
		friend VkCore;

	public:
		[[nodiscard]] VkImage Create(glm::ivec2 resolution,
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
			VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
			VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) const;
		void TransitionLayout(VkImage image, VkImageLayout oldLayout,
			VkImageLayout newLayout, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) const;
		void Destroy(VkImage image) const;

		[[nodiscard]] VkImageView CreateView(VkImage image, 
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) const;
		void DestroyView(VkImageView imageView) const;

		static void GetLayoutMasks(VkImageLayout layout, VkAccessFlags& outAccessFlags, 
			VkPipelineStageFlags& outPipelineStageFlags);

		[[nodiscard]] VkFormat FindSupportedFormat(const ArrayPtr<VkFormat>& candidates,
			VkImageTiling tiling, VkFormatFeatureFlags features) const;

	private:
		explicit VkImageHandler(VkCore& core);
	};
}
