#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkRenderPassHandler final : public VkHandler
	{
	public:
		struct CreateInfo final
		{
			bool useColorAttachment = true;
			VkAttachmentLoadOp colorLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			VkImageLayout colorInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkImageLayout colorFinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			bool useDepthAttachment = true;
			VkAttachmentStoreOp depthStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			VkImageLayout depthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			void* pNext = nullptr;
		};

		explicit VkRenderPassHandler(VkCore& core);

		/// <summary>Object that can be used to render a scene.</summary>
		[[nodiscard]] VkRenderPass Create(const CreateInfo& info = {}) const;
		void Begin(VkFramebuffer frameBuffer, VkRenderPass renderPass,
			glm::ivec2 offset, glm::ivec2 extent,
			VkClearValue* clearColors, uint32_t clearColorsCount) const;
		void End() const;
		void Destroy(VkRenderPass renderPass) const;
	};
}
