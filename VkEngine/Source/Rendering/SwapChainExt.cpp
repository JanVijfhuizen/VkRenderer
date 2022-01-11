#include "pch.h"
#include "Rendering/SwapChainExt.h"
#include "VkRenderer/VkCore/VkCore.h"
#include "Rendering/DescriptorPool.h"

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

	for (int32_t i = _deleteables.GetCount() - 1; i >= 0; --i)
	{
		auto& deleteable = _deleteables[i];
		if (deleteable.index == index)
			Delete(deleteable, false);
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
