#include "pch.h"
#include "VkHandlers/VkCommandBufferHandler.h"
#include "VkCore/VkCore.h"
#include "VkCore/VkCoreLogicalDevice.h"

namespace vi
{
	VkCommandBuffer VkCommandBufferHandler::Create() const
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = core.GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		const auto result = vkAllocateCommandBuffers(core.GetLogicalDevice(), &allocInfo, &commandBuffer);
		assert(!result);

		return commandBuffer;
	}

	void VkCommandBufferHandler::BeginRecording(const VkCommandBuffer commandBuffer)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		_current = commandBuffer;
	}

	void VkCommandBufferHandler::EndRecording()
	{
		const auto result = vkEndCommandBuffer(_current);
		assert(!result);
		_current = VK_NULL_HANDLE;
	}

	void VkCommandBufferHandler::Destroy(const VkCommandBuffer commandBuffer) const
	{
		vkFreeCommandBuffers(core.GetLogicalDevice(), core.GetCommandPool(), 1, &commandBuffer);
	}

	VkCommandBuffer VkCommandBufferHandler::GetCurrent() const
	{
		return _current;
	}

	void VkCommandBufferHandler::Submit(const SubmitInfo& info) const
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.waitSemaphoreCount = info.waitSemaphore ? 1 : 0;
		submitInfo.pWaitSemaphores = &info.waitSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.commandBufferCount = info.buffersCount;
		submitInfo.pCommandBuffers = info.buffers;
		submitInfo.signalSemaphoreCount = info.signalSemaphore ? 1 : 0;
		submitInfo.pSignalSemaphores = &info.signalSemaphore;

		if(info.fence)
			vkResetFences(core.GetLogicalDevice(), 1, &info.fence);
		const auto result = vkQueueSubmit(core.GetQueues().graphics, 1, &submitInfo, info.fence);
		assert(!result);
	}

	VkCommandBufferHandler::VkCommandBufferHandler(VkCore& core) : VkHandler(core)
	{
		
	}
}
