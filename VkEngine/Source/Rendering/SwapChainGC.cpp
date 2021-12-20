#include "pch.h"
#include "Rendering/SwapChainGC.h"
#include "Rendering/RenderManager.h"
#include "VkRenderer/VkRenderer.h"

SwapChainGC::~SwapChainGC()
{
	for (auto& deleteable : _deleteables)
		Delete(deleteable);
}

void SwapChainGC::Update()
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();
	auto& swapChain = renderer.GetSwapChain();

	const uint32_t index = swapChain.GetCurrentImageIndex();

	for (int32_t i = _deleteables.GetCount() - 1; i >= 0; --i)
	{
		auto& deleteable = _deleteables[i];
		if (deleteable.index == index)
			Delete(deleteable);
	}
}

void SwapChainGC::Enqueue(const VkBuffer buffer)
{
	Deleteable deleteable{};
	deleteable.buffer = buffer;
	deleteable.type = 0;
	Enqueue(deleteable);
}

void SwapChainGC::Enqueue(const VkSampler sampler)
{
	Deleteable deleteable{};
	deleteable.sampler = sampler;
	deleteable.type = 1;
	Enqueue(deleteable);
}

void SwapChainGC::Enqueue(Deleteable& deleteable)
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();
	auto& swapChain = renderer.GetSwapChain();

	deleteable.index = swapChain.GetCurrentImageIndex();

	_deleteables.Add(deleteable);
}

void SwapChainGC::Delete(Deleteable& deleteable)
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();

	switch (deleteable.type)
	{
		// VkBuffer
	case 0:
		renderer.DestroyBuffer(deleteable.buffer);
		break;
		// VkSampler
	case 1:
		renderer.DestroySampler(deleteable.sampler);
		break;
	default:
		break;
	}
}
