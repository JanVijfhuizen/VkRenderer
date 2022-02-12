#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkCore;

	class VkMemoryHandler final : public VkHandler
	{
	public:
		explicit VkMemoryHandler(VkCore& core);

		/// <returns>Memory requirements for target buffer.</returns>
		[[nodiscard]] VkMemoryRequirements GetRequirements(VkBuffer buffer) const;
		/// <returns>Memory requirements for target image.</returns>
		[[nodiscard]] VkMemoryRequirements GetRequirements(VkImage image) const;
		/// <param name="image">Image to allocate memory for.</param>
		/// <param name="flags">Memory flags.</param>
		/// <returns>Allocated memory on the GPU.</returns>
		[[nodiscard]] VkDeviceMemory Allocate(VkImage image, VkMemoryPropertyFlags flags) const;
		/// <param name="buffer">Buffer to allocate memory for.</param>
		/// <param name="flags">Memory flags.</param>
		/// <returns>Allocated memory on the GPU.</returns>
		[[nodiscard]] VkDeviceMemory Allocate(VkBuffer buffer, VkMemoryPropertyFlags flags) const;
		/// <param name="memRequirements">Requirements for the memory being allocated.</param>
		/// <param name="flags">Memory flags.</param>
		/// <returns>Allocated memory on the GPU.</returns>
		[[nodiscard]] VkDeviceMemory Allocate(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags flags) const;
		/// <summary>
		/// Bind an image to memory.
		/// </summary>
		/// <param name="image">Image to bind it to.</param>
		/// <param name="memory">Memory to bind to the image.</param>
		/// <param name="offset">Offset for the memory to bind.</param>
		void Bind(VkImage image, VkDeviceMemory memory, VkDeviceSize offset = 0) const;
		/// <summary>
		/// Bind a buffer to memory.
		/// </summary>
		/// <param name="buffer">Buffer to bind it to</param>
		/// <param name="memory">Memory to bind to the buffer.</param>
		/// <param name="offset">Offset for the memory to bind.</param>
		void Bind(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize offset = 0) const;
		template <typename T>
		/// <summary>
		/// Map a CPU memory range to a GPU memory range.
		/// </summary>
		/// <param name="memory">GPU memory location.</param>
		/// <param name="input">CPU memory range.</param>
		/// <param name="offset">GPU memory offset.</param>
		/// <param name="size">Size of transferable memory range.</param>
		void Map(VkDeviceMemory memory, T* input, VkDeviceSize offset, size_t size = sizeof(T));	
		void Free(VkDeviceMemory memory) const;

		/// <summary>
		/// Find the type of memory that can be used with target properties.
		/// </summary>
		/// <param name="typeFilter">Filters the type of memory.</param>
		/// <param name="properties">Required properties for the memory.</param>
		/// <returns></returns>
		[[nodiscard]] uint32_t FindType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	private:
		void IntMap(VkDeviceMemory memory, void* input, VkDeviceSize offset, size_t size) const;
	};

	template <typename T>
	void VkMemoryHandler::Map(
		const VkDeviceMemory memory, T* input, 
		const VkDeviceSize offset, const size_t size)
	{
		IntMap(memory, input, offset, size);
	}
}
