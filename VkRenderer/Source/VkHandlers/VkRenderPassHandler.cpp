#include "pch.h"
#include "VkHandlers/VkRenderPassHandler.h"
#include "VkCore/VkCore.h"

namespace vi
{
	VkRenderPass VkRenderPassHandler::Create(const CreateInfo& info) const
	{
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = info.useColorAttachment;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = info.useColorAttachment;
		subpass.pColorAttachments = &colorAttachmentRef;
		if (info.useDepthAttachment)
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = 0;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = 0;
		dependency.dstAccessMask = 0;

		const ArrayPtr<VkAttachmentDescription> descriptions{ 2, GMEM_TEMP };
		uint32_t descriptionsCount = 0;

		if (info.useColorAttachment)
		{
			auto& swapChain = core.GetSwapChain();
			const auto format = swapChain.GetFormat();

			auto& colorDescription = descriptions[descriptionsCount++];
			colorDescription.format = format;
			colorDescription.samples = VK_SAMPLE_COUNT_1_BIT;
			colorDescription.loadOp = info.colorLoadOp;
			colorDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorDescription.initialLayout = info.colorInitialLayout;
			colorDescription.finalLayout = info.colorFinalLayout;

			dependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		if (info.useDepthAttachment)
		{
			auto& depthDescription = descriptions[descriptionsCount++];
			depthDescription.format = core.GetSwapChain().GetDepthBufferFormat();
			depthDescription.samples = VK_SAMPLE_COUNT_1_BIT;
			depthDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthDescription.storeOp = info.depthStoreOp;
			depthDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthDescription.finalLayout = info.depthFinalLayout;

			dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = descriptionsCount;
		renderPassInfo.pAttachments = descriptions.GetData();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkRenderPass renderPass;
		const auto result = vkCreateRenderPass(core.GetLogicalDevice(), &renderPassInfo, nullptr, &renderPass);
		assert(!result);
		return renderPass;
	}

	void VkRenderPassHandler::Begin(
		const VkFramebuffer frameBuffer, 
		const VkRenderPass renderPass, 
		const glm::ivec2 offset,
		const glm::ivec2 extent, 
		VkClearValue* clearColors, 
		const uint32_t clearColorsCount) const
	{
		const VkExtent2D extentVk
		{
			static_cast<uint32_t>(extent.x),
			static_cast<uint32_t>(extent.y)
		};

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = frameBuffer;
		renderPassInfo.renderArea.offset = { offset.x, offset.y };
		renderPassInfo.renderArea.extent = extentVk;

		renderPassInfo.clearValueCount = clearColorsCount;
		renderPassInfo.pClearValues = clearColors;

		vkCmdBeginRenderPass(core.GetCommandBufferHandler().GetCurrent(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VkRenderPassHandler::End() const
	{
		vkCmdEndRenderPass(core.GetCommandBufferHandler().GetCurrent());
	}

	void VkRenderPassHandler::Destroy(const VkRenderPass renderPass) const
	{
		vkDestroyRenderPass(core.GetLogicalDevice(), renderPass, nullptr);
	}

	VkRenderPassHandler::VkRenderPassHandler(VkCore& core) : VkHandler(core)
	{

	}
}
