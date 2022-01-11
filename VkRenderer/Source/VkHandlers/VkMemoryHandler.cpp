#include "pch.h"
#include "VkHandlers/VkMemoryHandler.h"
#include "VkCore/VkCore.h"

namespace vi
{
	VkDeviceMemory VkMemoryHandler::Allocate(
		const VkImage image, 
		const VkMemoryPropertyFlags flags) const
	{
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(core.GetLogicalDevice(), image, &memRequirements);
		return Allocate(memRequirements, flags);
	}

	VkDeviceMemory VkMemoryHandler::Allocate(
		const VkBuffer buffer, 
		const VkMemoryPropertyFlags flags) const
	{
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(core.GetLogicalDevice(), buffer, &memRequirements);
		return Allocate(memRequirements, flags);
	}

	VkDeviceMemory VkMemoryHandler::Allocate(
		const VkMemoryRequirements memRequirements,
		const VkMemoryPropertyFlags flags) const
	{
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindType(memRequirements.memoryTypeBits, flags);

		VkDeviceMemory memory;
		const auto result = vkAllocateMemory(core.GetLogicalDevice(), &allocInfo, nullptr, &memory);
		assert(!result);
		return memory;
	}

	void VkMemoryHandler::Bind(const VkImage image, 
		const VkDeviceMemory memory, const VkDeviceSize offset) const
	{
		vkBindImageMemory(core.GetLogicalDevice(), image, memory, offset);
	}

	void VkMemoryHandler::Bind(const VkBuffer buffer, 
		const VkDeviceMemory memory, const VkDeviceSize offset) const
	{
		vkBindBufferMemory(core.GetLogicalDevice(), buffer, memory, offset);
	}

	void VkMemoryHandler::IntMap(const VkDeviceMemory memory, 
		void* input, const VkDeviceSize offset, const size_t size) const
	{
		const auto logicalDevice = core.GetLogicalDevice();

		void* data;
		vkMapMemory(logicalDevice, memory, offset, size, 0, &data);
		memcpy(data, static_cast<const void*>(input), size);
		vkUnmapMemory(logicalDevice, memory);
	}

	void VkMemoryHandler::Free(const VkDeviceMemory memory) const
	{
		vkFreeMemory(core.GetLogicalDevice(), memory, nullptr);
	}

	uint32_t VkMemoryHandler::FindType(
		const uint32_t typeFilter, 
		const VkMemoryPropertyFlags properties) const
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(core.GetPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			if (typeFilter & 1 << i)
			{
				const bool requiredPropertiesPresent = (memProperties.memoryTypes[i].propertyFlags & properties) == properties;
				if (!requiredPropertiesPresent)
					continue;

				return i;
			}

		throw std::exception("Memory type not available!");
	}

	VkMemoryHandler::VkMemoryHandler(VkCore& core) : VkHandler(core)
	{

	}
}
