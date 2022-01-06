#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkFrameBufferHandler final : public VkHandler
	{
		friend VkCore;

	public:
		[[nodiscard]] VkFramebuffer Create(const VkImageView* imageViews,
			uint32_t imageViewCount, VkRenderPass renderPass, VkExtent2D extent) const;
		void Destroy(VkFramebuffer frameBuffer) const;

	private:
		explicit VkFrameBufferHandler(VkCore& core);
	};
}
