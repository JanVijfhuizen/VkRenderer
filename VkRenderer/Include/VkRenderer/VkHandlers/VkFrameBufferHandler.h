#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkFrameBufferHandler final : public VkHandler
	{
		friend VkCore;

	public:
		struct CreateInfo final
		{
			// Image views to use.
			VkImageView* imageViews;
			uint32_t imageViewCount;
			// Render pass to use.
			VkRenderPass renderPass;
			// Resolution of the image.
			VkExtent2D extent;
		};

		/// <returns>Object that can be used as a render target.</returns>
		[[nodiscard]] VkFramebuffer Create(const CreateInfo& info) const;
		void Destroy(VkFramebuffer frameBuffer) const;

	private:
		explicit VkFrameBufferHandler(VkCore& core);
	};
}
