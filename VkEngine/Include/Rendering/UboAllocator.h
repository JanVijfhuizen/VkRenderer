#pragma once
#include "Rendering/VulkanRenderer.h"
#include "VkRenderer/VkHandlers/VkShaderHandler.h"
#include "VkRenderer/VkHandlers/VkMemoryHandler.h"
#include "VkRenderer/VkCore/VkCoreSwapchain.h"

/// <summary>
/// Manages a chunk of GPU memory and is able to create uniform sized buffers for it.
/// </summary>
template <typename T>
class UboAllocator final
{
public:
	/// <param name="bufferSize">Size of a single buffer.</param>
	explicit UboAllocator(VulkanRenderer& renderer, size_t bufferSize);
	~UboAllocator();

	// Create buffer for the managed memory.
	[[nodiscard]] VkBuffer CreateBuffer() const;
	// Get the managed memory.
	[[nodiscard]] VkDeviceMemory GetMemory() const;
	// Get the memory alignment requirement for the buffers.
	[[nodiscard]] size_t GetAlignment() const;
	[[nodiscard]] size_t GetOffset(uint32_t swapChainImageIndex) const;

private:
	VulkanRenderer& _renderer;
	VkDeviceMemory _memory;
	VkMemoryRequirements _bufferMemoryRequirements;
	size_t _bufferSize;
	size_t _blockSize;
};

template <typename T>
UboAllocator<T>::UboAllocator(VulkanRenderer& renderer, const size_t bufferSize) :
	_renderer(renderer), _bufferSize(bufferSize)
{
	auto& memoryHandler = renderer.GetMemoryHandler();
	auto& shaderHandler = renderer.GetShaderHandler();
	auto& swapChain = renderer.GetSwapChain();

	// Used to get the memory requirement bits.
	const auto tempBuffer = shaderHandler.CreateBuffer(sizeof(T) * bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	auto memRequirements = _bufferMemoryRequirements = memoryHandler.GetRequirements(tempBuffer);
	_blockSize = memRequirements.size;
	memRequirements.size *= swapChain.GetLength();
	_memory = memoryHandler.Allocate(memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	shaderHandler.DestroyBuffer(tempBuffer);
}

template <typename T>
UboAllocator<T>::~UboAllocator()
{
	auto& memoryHandler = _renderer.GetMemoryHandler();
	memoryHandler.Free(_memory);
}

template <typename T>
VkBuffer UboAllocator<T>::CreateBuffer() const
{
	auto& shaderHandler = _renderer.GetShaderHandler();
	return shaderHandler.CreateBuffer(sizeof(T) * _bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
}

template <typename T>
VkDeviceMemory UboAllocator<T>::GetMemory() const
{
	return _memory;
}

template <typename T>
size_t UboAllocator<T>::GetAlignment() const
{
	return _bufferMemoryRequirements.alignment;
}

template <typename T>
VkDeviceSize UboAllocator<T>::GetOffset(const uint32_t swapChainImageIndex) const
{
	return _blockSize * swapChainImageIndex;
}
