#pragma once
#include "Rendering/Renderer.h"

template <typename T>
class UboPool final
{
public:
	explicit UboPool(Renderer& renderer, size_t bufferSize, size_t capacity);
	~UboPool();

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
UboPool<T>::UboPool(Renderer& renderer, const size_t bufferSize, const size_t capacity) :
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
UboPool<T>::~UboPool()
{
	auto& memoryHandler = _renderer.GetMemoryHandler();
	memoryHandler.Free(_memory);
}

template <typename T>
VkBuffer UboPool<T>::CreateBuffer() const
{
	auto& shaderHandler = _renderer.GetShaderHandler();
	return shaderHandler.CreateBuffer(sizeof(T) * _bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
}

template <typename T>
VkDeviceMemory UboPool<T>::GetMemory() const
{
	return _memory;
}

template <typename T>
size_t UboPool<T>::GetAlignment() const
{
	return _bufferMemoryRequirements.alignment;
}
