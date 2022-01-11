#pragma once
#include "Rendering/Renderer.h"

template <typename T>
class UboPool final
{
public:
	struct Instance final
	{
		friend UboPool;

	private:
		VkBuffer _buffer;
		size_t _offset;
	};

	explicit UboPool(Renderer& renderer, size_t size, VkMemoryPropertyFlagBits flags);
	~UboPool();

private:
	Renderer& _renderer;
	VkDeviceMemory _memory;
	vi::Vector<Instance> _instances;
};

template <typename T>
UboPool<T>::UboPool(Renderer& renderer, const size_t size, const VkMemoryPropertyFlagBits flags) :
	_renderer(renderer)
{
	auto& memoryHandler = renderer.GetMemoryHandler();
	auto& shaderHandler = renderer.GetShaderHandler();

	// Used to get the memory requirement bits.
	const auto tempBuffer = shaderHandler.CreateBuffer(sizeof(T), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	auto memRequirements = memoryHandler.GetRequirements(tempBuffer);
	memRequirements.size = sizeof(T) * size;
	_memory = memoryHandler.Allocate(memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	_instances = { size, GMEM };
}

template <typename T>
UboPool<T>::~UboPool()
{
	auto& swapChainExt = _renderer.GetSwapChainExt();
	for (auto& instance : _instances)
		swapChainExt.Collect(instance);
	swapChainExt.Collect(_memory);
}
