#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkFrameBufferHandler final : public VkHandler
	{
	public:
		struct CreateInfo final
		{
			// Image views to use.
			VkImageView* imageViews;
			uint32_t imageViewCount;
			// Render pass to use.
			VkRenderPass renderPass;
			// Resolution of the image.
			glm::ivec2 extent;
			// Number of layers.
			uint32_t layerCount = 1;
		};

		explicit VkFrameBufferHandler(VkCore& core);

		/// <returns>Object that can be used as a render target.</returns>
		[[nodiscard]] VkFramebuffer Create(const CreateInfo& info) const;
		void Destroy(VkFramebuffer frameBuffer) const;
	};
}
