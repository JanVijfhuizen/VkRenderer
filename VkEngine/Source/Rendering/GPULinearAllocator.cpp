#include "pch.h"
#include "Rendering/GPULinearAllocator.h"
#include "VkRenderer/VkCore/VkCore.h"

GPULinearAllocator::GPULinearAllocator(vi::VkCore& core) : VkHandler(core)
{
}

Memory GPULinearAllocator::Allocate(const VkImage image, const VkMemoryPropertyFlags flags) const
{
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(core.GetLogicalDevice(), image, &memRequirements);
	return Allocate(memRequirements, flags);
}

Memory GPULinearAllocator::Allocate(const VkBuffer buffer, const VkMemoryPropertyFlags flags) const
{
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(core.GetLogicalDevice(), buffer, &memRequirements);
	return Allocate(memRequirements, flags);
}

Memory GPULinearAllocator::Allocate(const VkMemoryRequirements memRequirements, const VkMemoryPropertyFlags flags) const
{
	auto& memoryHandler = core.GetMemoryHandler();

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memoryHandler.FindType(memRequirements.memoryTypeBits, flags);

	VkDeviceMemory memory;
	const auto result = vkAllocateMemory(core.GetLogicalDevice(), &allocInfo, nullptr, &memory);
	assert(!result);
	return {};
}
