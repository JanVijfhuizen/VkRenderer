#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkSyncHandler final : public VkHandler
	{
		friend VkCore;

	public:
		/// <returns>Object that can act as a wait handle for GPU operations.</returns>
		[[nodiscard]] VkSemaphore CreateSemaphore() const;
		void DestroySemaphore(VkSemaphore semaphore) const;

		/// <returns>Object that can act as a wait handle for CPU operations.</returns>
		[[nodiscard]] VkFence CreateFence() const;
		void WaitForFence(VkFence fence) const;
		void DestroyFence(VkFence fence) const;
		
	private:
		explicit VkSyncHandler(VkCore& core);
	};
}