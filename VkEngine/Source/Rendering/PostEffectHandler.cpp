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
}

void PostEffectHandler::DestroySwapChainAssets() const
{
	auto& renderPassHandler = _renderer.GetRenderPassHandler();
	renderPassHandler.Destroy(_renderPass);
}
