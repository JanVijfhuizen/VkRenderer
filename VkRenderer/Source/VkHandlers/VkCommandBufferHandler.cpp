#include "pch.h"
#include "VkHandlers/VkCommandBufferHandler.h"
#include "VkCore/VkCore.h"

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

	VkCommandBufferHandler::VkCommandBufferHandler(VkCore& core) : VkHandler(core)
	{
		
	}
}