#include "pch.h"
#include "Rendering/PostEffectHandler.h"
#include "VkRenderer/VkHandlers/VkRenderPassHandler.h"
#include "Rendering/Renderer.h"

PostEffectHandler::PostEffectHandler(Renderer& renderer) : VkHandler(renderer), Dependency(renderer), _renderer(renderer)
{
	OnRecreateSwapChainAssets();
}

PostEffectHandler::~PostEffectHandler()
{
	auto& renderPassHandler = core.GetRenderPassHandler();
	renderPassHandler.Destroy(_renderPass);
}

VkRenderPass PostEffectHandler::GetRenderPass() const
{
	return _renderPass;
}

VkExtent2D PostEffectHandler::GetExtent() const
{
	return _extent;
}

void PostEffectHandler::OnRecreateSwapChainAssets()
{
	if (_renderPass)
		DestroySwapChainAssets();

	auto& imageHandler = _renderer.GetImageHandler();
	auto& memoryHandler = _renderer.GetMemoryHandler();
	auto& renderPassHandler = _renderer.GetRenderPassHandler();
	auto& swapChainHandler = _renderer.GetSwapChain();

	vi::VkRenderPassHandler::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.useColorAttachment = true;
	renderPassCreateInfo.colorFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	renderPassCreateInfo.useDepthAttachment = true;
	renderPassCreateInfo.depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	renderPassCreateInfo.depthFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	_renderPass = renderPassHandler.Create(renderPassCreateInfo);
	_extent = swapChainHandler.GetExtent();

	for (auto& frame : _frames)
	{
		vi::VkImageHandler::CreateInfo colorImageCreateInfo{};
		colorImageCreateInfo.resolution = { _extent.width, _extent.height };
		colorImageCreateInfo.format = swapChainHandler.GetFormat();
		colorImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		frame.colorImage = imageHandler.Create(colorImageCreateInfo);

		vi::VkImageHandler::CreateInfo depthImageCreateInfo{};
		depthImageCreateInfo.resolution = { _extent.width, _extent.height };
		depthImageCreateInfo.format = swapChainHandler.GetDepthBufferFormat();
		depthImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		frame.depthImage = imageHandler.Create(depthImageCreateInfo);

		frame.colorMemory = memoryHandler.Allocate(frame.colorImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		frame.depthMemory = memoryHandler.Allocate(frame.depthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		memoryHandler.Bind(frame.colorImage, frame.colorMemory);
		memoryHandler.Bind(frame.depthImage, frame.depthMemory);
	}
}

void PostEffectHandler::DestroySwapChainAssets() const
{
	auto& imageHandler = _renderer.GetImageHandler();
	auto& memoryHandler = _renderer.GetMemoryHandler();
	auto& renderPassHandler = _renderer.GetRenderPassHandler();

	renderPassHandler.Destroy(_renderPass);

	for (auto& frame : _frames)
	{
		imageHandler.DestroyView(frame.colorView);
		imageHandler.DestroyView(frame.depthView);

		imageHandler.Destroy(frame.colorImage);
		imageHandler.Destroy(frame.depthImage);

		memoryHandler.Free(frame.colorMemory);
		memoryHandler.Free(frame.depthMemory);
	}
}
