#include "pch.h"
#include "Rendering/SwapChainExt.h"
#include "VkRenderer/VkCore/VkCore.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/VulkanRenderer.h"
#include "VkRenderer/VkHandlers/VkShaderHandler.h"
#include "VkRenderer/VkHandlers/VkMemoryHandler.h"
#include "VkRenderer/VkHandlers/VkImageHandler.h"
#include "VkRenderer/VkHandlers/VkFrameBufferHandler.h"
#include "VkRenderer/VkCore/VkCoreSwapchain.h"

SwapChainExt::Dependency::Dependency(VulkanRenderer& renderer) : renderer(renderer)
{
	auto& swapChainExt = renderer.GetSwapChainExt();
	swapChainExt._dependencies.Add(this);
}

SwapChainExt::Dependency::~Dependency()
{
	auto& swapChainExt = renderer.GetSwapChainExt();
	swapChainExt._dependencies.Remove(this);
}

SwapChainExt::SwapChainExt(vi::VkCore& core) : VkHandler(core)
{

}

SwapChainExt::~SwapChainExt()
{
	for (auto& deleteable : _deleteables)
		Delete(deleteable, true);
}

void SwapChainExt::Update()
{
	auto& swapChain = core.GetSwapChain();
	const uint32_t index = swapChain.GetImageIndex();

	// Check all objects in the garbage collection and delete those that have made a full rotation.
	// In other words, delete those that have passed every swap chain image to make sure that we don't delete
	// anything that might still be used for images in flight.
	for (int32_t i = _deleteables.GetCount() - 1; i >= 0; --i)
	{
		auto& deleteable = _deleteables[i];

		// If the image has been deleted while this image was available.
		if (deleteable.index == index)
		{
			// If the image was created THIS frame, continue.
			if(!deleteable.looped)
			{
				deleteable.looped = true;
				continue;
			}

			Delete(deleteable, false);
			_deleteables.RemoveAt(i);
		}
	}

	// Check if the swap chain has been recreated, and if so, trigger the recreation event for all subscribers.
	if(swapChain.GetShouldRecreateAssets())
	{
		swapChain.Reconstruct();
		for (auto& dependency : _dependencies)
			dependency->OnRecreateSwapChainAssets();
	}
}

void SwapChainExt::Collect(const VkBuffer buffer)
{
	Deleteable deleteable{};
	deleteable.buffer = buffer;
	deleteable.type = Deleteable::Type::buffer;
	Collect(deleteable);
}

void SwapChainExt::Collect(const VkSampler sampler)
{
	Deleteable deleteable{};
	deleteable.sampler = sampler;
	deleteable.type = Deleteable::Type::sampler;
	Collect(deleteable);
}

void SwapChainExt::Collect(const VkDeviceMemory memory)
{
	Deleteable deleteable{};
	deleteable.memory = memory;
	deleteable.type = Deleteable::Type::memory;
	Collect(deleteable);
}

void SwapChainExt::Collect(const VkImage image)
{
	Deleteable deleteable{};
	deleteable.image = image;
	deleteable.type = Deleteable::Type::image;
	Collect(deleteable);
}

void SwapChainExt::Collect(const VkImageView imageView)
{
	Deleteable deleteable{};
	deleteable.imageView = imageView;
	deleteable.type = Deleteable::Type::imageView;
	Collect(deleteable);
}

void SwapChainExt::Collect(const VkFramebuffer framebuffer)
{
	Deleteable deleteable{};
	deleteable.framebuffer = framebuffer;
	deleteable.type = Deleteable::Type::framebuffer;
	Collect(deleteable);
}

void SwapChainExt::Collect(const VkDescriptorSet descriptor, DescriptorPool& pool)
{
	Deleteable deleteable{};
	deleteable.descriptor = { descriptor, &pool };
	deleteable.type = Deleteable::Type::descriptor;
	Collect(deleteable);
}

void SwapChainExt::Collect(Deleteable& deleteable)
{
	auto& swapChain = core.GetSwapChain();
	deleteable.index = swapChain.GetImageIndex();

	_deleteables.Add(deleteable);
}

void SwapChainExt::Delete(Deleteable& deleteable, const bool calledByDetructor) const
{
	auto& frameBufferHandler = core.GetFrameBufferHandler();
	auto& imageHandler = core.GetImageHandler();
	auto& memoryHandler = core.GetMemoryHandler();
	auto& shaderHandler = core.GetShaderHandler();

	// It's not the most readable, but a switch case + a union is pretty fast.
	switch (deleteable.type)
	{
	case Deleteable::Type::buffer:
		shaderHandler.DestroyBuffer(deleteable.buffer);
		break;
	case Deleteable::Type::sampler:
		shaderHandler.DestroySampler(deleteable.sampler);
		break;
	case Deleteable::Type::memory:
		memoryHandler.Free(deleteable.memory);
		break;
	case Deleteable::Type::image:
		imageHandler.Destroy(deleteable.image);
		break;
	case Deleteable::Type::imageView:
		imageHandler.DestroyView(deleteable.imageView);
		break;
	case Deleteable::Type::framebuffer:
		frameBufferHandler.Destroy(deleteable.framebuffer);
		break;
	case Deleteable::Type::descriptor:
		if (!calledByDetructor)
			deleteable.descriptor.pool->Add(deleteable.descriptor.set);
		break;
	default:
		break;
	}
}
