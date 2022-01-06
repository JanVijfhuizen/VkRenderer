#include "pch.h"
#include "VkHandlers/VkImageHandler.h"
#include "VkCore/VkCore.h"

namespace vi
{
	VkImageView VkImageHandler::CreateView(const VkImage image, 
		const VkFormat format, const VkImageAspectFlags aspectFlags) const
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		const auto result = vkCreateImageView(core.GetLogicalDevice(), &createInfo, nullptr, &imageView);
		assert(!result);
		return imageView;
	}

	void VkImageHandler::DestroyView(VkImageView imageView) const
	{
		vkDestroyImageView(core.GetLogicalDevice(), imageView, nullptr);
	}

	VkImageHandler::VkImageHandler(VkCore& core) : VkHandler(core)
	{
	}
}
