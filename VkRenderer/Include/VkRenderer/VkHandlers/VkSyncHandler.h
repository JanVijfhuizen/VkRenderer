#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkSyncHandler final : public VkHandler
	{
		friend VkCore;

	public:
		[[nodiscard]] VkSemaphore CreateSemaphore() const;
		void DestroySemaphore(VkSemaphore semaphore) const;

		[[nodiscard]] VkFence CreateFence() const;
		void WaitForFence(VkFence fence) const;
		void DestroyFence(VkFence fence) const;
		
	private:
		explicit VkSyncHandler(VkCore& core);
	};
}