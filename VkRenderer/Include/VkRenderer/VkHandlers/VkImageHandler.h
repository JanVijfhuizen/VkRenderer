#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkImageHandler final : public VkHandler
	{
		friend VkCore;

	public:
		/// <param name="resolution">The resolution of the image.</param>
		/// <param name="format">The format of the image (i.e. (S)RGB).</param>
		/// <param name="tiling">How should the image function when going outside the borders.</param>
		/// <param name="usage">What the image will be used for.</param>
		/// <returns>Object that can be used as textures, depth buffers, render targets and more.</returns>
		[[nodiscard]] VkImage Create(glm::ivec2 resolution,
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
			VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
			VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) const;
		/// <summary>
		/// Transition the layout of an image. Used when the application of the image changes (render target to texture for instance).
		/// </summary>
		/// <param name="image">Image to be transitioned.</param>
		/// <param name="aspectFlags">Image aspects.</param>
		void TransitionLayout(VkImage image, VkImageLayout oldLayout,
			VkImageLayout newLayout, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) const;
		void Destroy(VkImage image) const;

		/// <param name="image">Image to read from.</param>
		/// <param name="format">Format of the image.</param>
		/// <param name="aspectFlags">Image aspect flags.</param>
		/// <returns>Object that can read an image a certain way.</returns>
		[[nodiscard]] VkImageView CreateView(VkImage image, 
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) const;
		void DestroyView(VkImageView imageView) const;

		/// <summary>
		/// Returns the layout masks needed to transition an image layout.
		/// </summary>
		/// <param name="layout">Layout from which to get the flags.</param>
		/// <param name="outAccessFlags">Out image flags.</param>
		/// <param name="outPipelineStageFlags">Out pipeline stage flags.</param>
		static void GetLayoutMasks(VkImageLayout layout, VkAccessFlags& outAccessFlags, 
			VkPipelineStageFlags& outPipelineStageFlags);

		/// <param name="candidates">List of possible candidates</param>
		/// <param name="tiling">How should the image function when going outside the borders.</param>
		/// <param name="features">Required features for the chosen format.</param>
		/// <returns>A supported format if found, otherwise returns null.</returns>
		[[nodiscard]] VkFormat FindSupportedFormat(const ArrayPtr<VkFormat>& candidates,
			VkImageTiling tiling, VkFormatFeatureFlags features) const;

	private:
		explicit VkImageHandler(VkCore& core);
	};
}
