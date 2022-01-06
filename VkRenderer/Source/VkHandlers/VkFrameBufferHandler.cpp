#include "pch.h"
#include "VkHandlers/VkFrameBufferHandler.h"
#include "VkCore/VkCore.h"

namespace vi
{
	VkFramebuffer VkFrameBufferHandler::Create(
		const VkImageView* imageViews, 
		const uint32_t imageViewCount,
		const VkRenderPass renderPass, 
		const VkExtent2D extent) const
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = imageViewCount;
		framebufferInfo.pAttachments = imageViews;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		VkFramebuffer frameBuffer;
		const auto result = vkCreateFramebuffer(core.GetLogicalDevice(), &framebufferInfo, nullptr, &frameBuffer);
		assert(!result);
		return frameBuffer;
	}

	void VkFrameBufferHandler::Destroy(const VkFramebuffer frameBuffer) const
	{
		vkDestroyFramebuffer(core.GetLogicalDevice(), frameBuffer, nullptr);
	}

	VkFrameBufferHandler::VkFrameBufferHandler(VkCore& core) : VkHandler(core)
	{

	}
}
