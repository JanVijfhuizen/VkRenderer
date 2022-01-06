#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkCommandBufferHandler final : public VkHandler
	{
		friend VkCore;

	public:
		[[nodiscard]] VkCommandBuffer Create() const;
		void BeginRecording(VkCommandBuffer commandBuffer);
		void EndRecording();
		void Destroy(VkCommandBuffer commandBuffer) const;
		[[nodiscard]] VkCommandBuffer GetCurrent() const;

	private:
		VkCommandBuffer _current = VK_NULL_HANDLE;

		explicit VkCommandBufferHandler(VkCore& core);
	};
}
