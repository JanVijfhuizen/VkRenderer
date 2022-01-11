#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"

struct Memory final
{
	VkDeviceMemory memory;
	VkDeviceSize offset;
	VkDeviceSize size;
};

class GPULinearAllocator : public vi::VkHandler
{
public:
	explicit GPULinearAllocator(vi::VkCore& core);

private:
	struct Block final
	{
		VkDeviceMemory memory;
		uint32_t memoryTypeBits;
		VkMemoryPropertyFlags properties;
	};
};
