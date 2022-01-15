#include "pch.h"
#include "VkHandlers/VkFrameBufferHandler.h"
#include "VkCore/VkCore.h"

namespace vi
{
	VkFramebuffer VkFrameBufferHandler::Create(const CreateInfo& info) const
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = info.renderPass;
		framebufferInfo.attachmentCount = info.imageViewCount;
		framebufferInfo.pAttachments = info.imageViews;
		framebufferInfo.width = info.extent.width;
		framebufferInfo.height = info.extent.height;
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
