#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkCommandBufferHandler final : public VkHandler
	{
		friend VkCore;

	public:
		struct SubmitInfo final
		{
			VkCommandBuffer* buffers;
			uint32_t buffersCount;

			VkSemaphore waitSemaphore = VK_NULL_HANDLE;
			VkSemaphore signalSemaphore = VK_NULL_HANDLE;
			VkFence fence = VK_NULL_HANDLE;
		};

		/// <returns>Handle that can be used to record and execute render commands, like drawing.</returns>
		[[nodiscard]] VkCommandBuffer Create() const;
		/// <returns>Override recording for target command buffer.</returns>
		void BeginRecording(VkCommandBuffer commandBuffer);
		/// <returns>End recording for current command buffer.</returns>
		void EndRecording();
		void Destroy(VkCommandBuffer commandBuffer) const;
		/// <returns>Get the command buffer that is currently being recorded.</returns>
		[[nodiscard]] VkCommandBuffer GetCurrent() const;

		/// <returns>Submit any number of command buffers to be executed.</returns>
		void Submit(const SubmitInfo& info) const;

	private:
		VkCommandBuffer _current = VK_NULL_HANDLE;

		explicit VkCommandBufferHandler(VkCore& core);
	};
}
