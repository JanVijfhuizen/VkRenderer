#include "pch.h"
#include "Rendering/SwapChainGC.h"
#include "Rendering/RenderSystem.h"
#include "VkRenderer/VkRenderer.h"

void SwapChainGC::Update()
{
	auto& renderSystem = RenderSystem::Get();
	auto& vkRenderer = renderSystem.GetVkRenderer();
	auto& swapChain = vkRenderer.GetSwapChain();

	const uint32_t imageIndex = swapChain.GetCurrentImageIndex();
	auto& queue = _queues[imageIndex];
	Free(queue);
}

void SwapChainGC::FreeAll()
{
	for (auto [index, queue] : _queues)
		Free(queue);
}

void SwapChainGC::Collect(const VkBuffer buffer, const uint32_t imageIndex)
{
	_queues[imageIndex].buffers.push_back(buffer);
}

void SwapChainGC::Collect(const VkSampler sampler, const uint32_t imageIndex)
{
	_queues[imageIndex].samplers.push_back(sampler);
}

void SwapChainGC::Free(ImageQueue& queue)
{
	auto& renderSystem = RenderSystem::Get();
	auto& vkRenderer = renderSystem.GetVkRenderer();

	for (const auto& buffer : queue.buffers)
		vkRenderer.DestroyBuffer(buffer);
	for (const auto& sampler : queue.samplers)
		vkRenderer.DestroySampler(sampler);
	queue.buffers.clear();
	queue.samplers.clear();
}
