#include "pch.h"
#include "VkRenderer/VkCore/VkCoreInfo.h"
#include "Rendering/MeshHandler.h"
#include "Rendering/PostEffectHandler.h"
#include "Rendering/VulkanRenderer.h"
#include "Rendering/TextureHandler.h"
#include "Rendering/SwapChainExt.h"

VulkanRenderer::VulkanRenderer(vi::VkCoreInfo& info, const Info& addInfo) : VkCore(info)
{
	_meshHandler = GMEM.New<MeshHandler>(*this);
	_shaderExt = GMEM.New<ShaderExt>(*this);
	_textureHandler = GMEM.New<TextureHandler>(*this);
	_swapChainExt = GMEM.New<SwapChainExt>(*this);
	_postEffectHandler = GMEM.New<PostEffectHandler>(*this, addInfo.msaaSamples);
}

VulkanRenderer::~VulkanRenderer()
{
	GMEM.Delete(_postEffectHandler);
	GMEM.Delete(_textureHandler);
	GMEM.Delete(_shaderExt);
	GMEM.Delete(_meshHandler);
	GMEM.Delete(_swapChainExt);
}

MeshHandler& VulkanRenderer::GetMeshHandler() const
{
	return *_meshHandler;
}

ShaderExt& VulkanRenderer::GetShaderExt() const
{
	return *_shaderExt;
}

SwapChainExt& VulkanRenderer::GetSwapChainExt() const
{
	return *_swapChainExt;
}

TextureHandler& VulkanRenderer::GetTextureHandler() const
{
	return *_textureHandler;
}

PostEffectHandler& VulkanRenderer::GetPostEffectHandler() const
{
	return *_postEffectHandler;
}
