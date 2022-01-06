#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkFrameBufferHandler final : public VkHandler
	{
		friend VkCore;

	public:
		/// <param name="imageViews">Image views to use.</param>
		/// <param name="renderPass">Render pass to use.</param>
		/// <param name="extent">Resolution of the image.</param>
		/// <returns>Object that can be used as a render target.</returns>
		[[nodiscard]] VkFramebuffer Create(const VkImageView* imageViews,
			uint32_t imageViewCount, VkRenderPass renderPass, VkExtent2D extent) const;
		void Destroy(VkFramebuffer frameBuffer) const;

	private:
		explicit VkFrameBufferHandler(VkCore& core);
	};
}
