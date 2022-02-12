#pragma once
#include "Rendering/Renderer.h"
#include "VkRenderer/VkHandlers/VkShaderHandler.h"
#include "VkRenderer/VkHandlers/VkMemoryHandler.h"

template <typename T>
class UboAllocator final
{
public:
	explicit UboAllocator(Renderer& renderer, size_t bufferSize, size_t capacity);
	~UboAllocator();

	[[nodiscard]] VkBuffer CreateBuffer() const;
	[[nodiscard]] VkDeviceMemory GetMemory() const;

	[[nodiscard]] size_t GetAlignment() const;

private:
	Renderer& _renderer;
	VkDeviceMemory _memory;
	VkMemoryRequirements _bufferMemoryRequirements;
	size_t _bufferSize;
};

template <typename T>
UboAllocator<T>::UboAllocator(Renderer& renderer, const size_t bufferSize, const size_t capacity) :
	_renderer(renderer), _bufferSize(bufferSize)
{
	auto& memoryHandler = renderer.GetMemoryHandler();
	auto& shaderHandler = renderer.GetShaderHandler();

	// Used to get the memory requirement bits.
	const auto tempBuffer = shaderHandler.CreateBuffer(sizeof(T) * bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	auto memRequirements = _bufferMemoryRequirements = memoryHandler.GetRequirements(tempBuffer);
	memRequirements.size *= capacity;
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
