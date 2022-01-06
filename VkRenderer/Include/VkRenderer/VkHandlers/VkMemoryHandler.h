#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkMemoryHandler final : public VkHandler
	{
		friend VkCore;

	public:
		[[nodiscard]] VkDeviceMemory Allocate(VkImage image, VkMemoryPropertyFlags flags) const;
		[[nodiscard]] VkDeviceMemory Allocate(VkBuffer buffer, VkMemoryPropertyFlags flags) const;
		[[nodiscard]] VkDeviceMemory Allocate(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags flags) const;
		void Bind(VkImage image, VkDeviceMemory memory, VkDeviceSize offset = 0) const;
		void Bind(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize offset = 0) const;
		template <typename T>
		void Map(VkDeviceMemory memory, T* input, VkDeviceSize offset, size_t size = sizeof(T));	
		void Free(VkDeviceMemory memory) const;

		[[nodiscard]] uint32_t FindType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	private:
		explicit VkMemoryHandler(VkCore& core);

		void ManualMap(VkDeviceMemory memory, void* input, VkDeviceSize offset, size_t size) const;
	};

	template <typename T>
	void VkMemoryHandler::Map(
		const VkDeviceMemory memory, T* input, 
		const VkDeviceSize offset, const size_t size)
	{
		ManualMap(memory, input, offset, size);
	}
}
