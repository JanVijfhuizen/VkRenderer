#include "pch.h"
#include "VkHandlers/VkSyncHandler.h"
#include "VkCore/VkCore.h"

namespace vi
{
	VkSemaphore VkSyncHandler::CreateSemaphore() const
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkSemaphore semaphore;
		const auto result = vkCreateSemaphore(core.GetLogicalDevice(), &semaphoreInfo, nullptr, &semaphore);
		assert(!result);
		return semaphore;
	}

	void VkSyncHandler::DestroySemaphore(const VkSemaphore semaphore) const
	{
		vkDestroySemaphore(core.GetLogicalDevice(), semaphore, nullptr);
	}

	VkFence VkSyncHandler::CreateFence() const
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkFence fence;
		const auto result = vkCreateFence(core.GetLogicalDevice(), &fenceInfo, nullptr, &fence);
		assert(!result);
		return fence;
	}

	void VkSyncHandler::WaitForFence(const VkFence fence) const
	{
		vkWaitForFences(core.GetLogicalDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
	}

	void VkSyncHandler::DestroyFence(const VkFence fence) const
	{
		vkDestroyFence(core.GetLogicalDevice(), fence, nullptr);
	}

	VkSyncHandler::VkSyncHandler(VkCore& core) : VkHandler(core)
	{
	}
}
