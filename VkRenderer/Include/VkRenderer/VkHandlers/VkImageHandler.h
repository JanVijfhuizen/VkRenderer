#pragma once
#include "VkHandler.h"

namespace vi
{
	/// <summary>
	/// Contains some core functionality for vulkan images and views.
	/// </summary>
	class VkImageHandler final : public VkHandler
	{
	public:
		/// <summary>
		/// Struct that contains information when creating an image.
		/// </summary>
		struct CreateInfo final
		{
			// The resolution of the image.
			glm::ivec2 resolution;
			// The depth of the mipmap chain.
			uint32_t mipLevels = 1;
			// The amount of sub images.
			uint32_t arrayLayers = 1;
			// The format of the image (i.e. (S)RGB).
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
			// How should the image function when going outside the borders.
			VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
			// What the image will be used for.
			VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			// Multisampling.
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
			// Flags.
			VkImageCreateFlags flags{};
		};

		/// <summary>
		/// Struct that contains information when creating an image view.
		/// </summary>
		struct ViewCreateInfo final
		{
			// Image to read from.
			VkImage image;
			// Type of image view.
			VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
			// The amount of layers in the image.
			uint32_t layerCount = 1;
			// The depth of the mipmap chain.
			uint32_t mipLevels = 1;
			// Format of the image.
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
			// Image aspect flags.
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		};

		/// <summary>
		/// Struct that contains information when transitioning the layout of an image.
		/// </summary>
		struct TransitionInfo final
		{
			// Image to be transitioned.
			VkImage image;
			VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkImageLayout newLayout;
			// The amount of layers.
			uint32_t layerCount = 1;
			// The depth of the mipmap chain.
			uint32_t mipLevels = 1;
			// Image aspects.
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		};

		explicit VkImageHandler(VkCore& core);

		/// <returns>Object that can be used as textures, depth buffers, render targets and more.</returns>
		[[nodiscard]] VkImage Create(const CreateInfo& info) const;
		/// <summary>
		/// Transition the layout of an image. Used when the application of the image changes (render target to texture for instance).
		/// </summary>
		void TransitionLayout(const TransitionInfo& info) const;
		void Destroy(VkImage image) const;

		/// <returns>Object that can read an image a certain way.</returns>
		[[nodiscard]] VkImageView CreateView(const ViewCreateInfo& info) const;
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
	};
}
