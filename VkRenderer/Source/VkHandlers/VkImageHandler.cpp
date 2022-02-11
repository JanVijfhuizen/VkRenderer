#include "pch.h"
#include "VkHandlers/VkImageHandler.h"
#include "VkCore/VkCore.h"

namespace vi
{
	VkImage VkImageHandler::Create(const CreateInfo& info) const
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(info.resolution.x);
		imageInfo.extent.height = static_cast<uint32_t>(info.resolution.y);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = info.mipLevels;
		imageInfo.arrayLayers = info.arrayLayers;
		imageInfo.format = info.format;
		imageInfo.tiling = info.tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = info.usage;
		imageInfo.samples = info.samples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = info.flags;

		VkImage image;
		const auto result = vkCreateImage(core.GetLogicalDevice(), &imageInfo, nullptr, &image);
		assert(!result);
		return image;
	}

	void VkImageHandler::TransitionLayout(const TransitionInfo& info) const
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = info.oldLayout;
		barrier.newLayout = info.newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = info.image;
		barrier.subresourceRange.aspectMask = info.aspectFlags;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = info.mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = info.layerCount;

		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;

		GetLayoutMasks(info.oldLayout, barrier.srcAccessMask, srcStage);
		GetLayoutMasks(info.newLayout, barrier.dstAccessMask, dstStage);

		vkCmdPipelineBarrier(core.GetCommandBufferHandler().GetCurrent(),
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	void VkImageHandler::Destroy(const VkImage image) const
	{
		vkDestroyImage(core.GetLogicalDevice(), image, nullptr);
	}

	VkImageView VkImageHandler::CreateView(const ViewCreateInfo& info) const
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = info.image;

		createInfo.viewType = info.viewType;
		createInfo.format = info.format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = info.aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = info.mipLevels;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = info.layerCount;

		VkImageView imageView;
		const auto result = vkCreateImageView(core.GetLogicalDevice(), &createInfo, nullptr, &imageView);
		assert(!result);
		return imageView;
	}

	void VkImageHandler::DestroyView(const VkImageView imageView) const
	{
		vkDestroyImageView(core.GetLogicalDevice(), imageView, nullptr);
	}

	void VkImageHandler::GetLayoutMasks(
		const VkImageLayout layout, 
		VkAccessFlags& outAccessFlags,
		VkPipelineStageFlags& outPipelineStageFlags)
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			outAccessFlags = 0;
			outPipelineStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			outAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
			outPipelineStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			outAccessFlags = VK_ACCESS_SHADER_READ_BIT;
			outPipelineStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			outAccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			outPipelineStageFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			break;
		default:
			throw std::exception("Layout transition not supported!");
		}
	}

	VkFormat VkImageHandler::FindSupportedFormat(
		const ArrayPtr<VkFormat>& candidates, 
		const VkImageTiling tiling,
		VkFormatFeatureFlags features) const
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(core.GetPhysicalDevice(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
				return format;
			if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
				return format;
		}

		throw std::exception("Format not available!");
	}

	VkImageHandler::VkImageHandler(VkCore& core) : VkHandler(core)
	{
	}
}
