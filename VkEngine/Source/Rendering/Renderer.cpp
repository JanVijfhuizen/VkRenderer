#include "pch.h"
#include "VkRenderer/VkCore/VkCoreInfo.h"
#include "Rendering/MeshHandler.h"
#include "Rendering/PostEffectHandler.h"
#include "Rendering/Renderer.h"
#include "Rendering/TextureHandler.h"
#include "Rendering/SwapChainExt.h"

Renderer::Renderer(vi::VkCoreInfo& info, const Info& addInfo) : VkCore(info)
{
	_meshHandler = GMEM.New<MeshHandler>(*this);
	_shaderExt = GMEM.New<ShaderExt>(*this);
	_textureHandler = GMEM.New<TextureHandler>(*this);
	_swapChainExt = GMEM.New<SwapChainExt>(*this);
	_postEffectHandler = GMEM.New<PostEffectHandler>(*this, addInfo.msaaSamples);
}

Renderer::~Renderer()
{
	GMEM.Delete(_postEffectHandler);
	GMEM.Delete(_textureHandler);
	GMEM.Delete(_shaderExt);
	GMEM.Delete(_meshHandler);
	GMEM.Delete(_swapChainExt);
}

MeshHandler& Renderer::GetMeshHandler() const
{
	return *_meshHandler;
}

ShaderExt& Renderer::GetShaderExt() const
{
	return *_shaderExt;
}

SwapChainExt& Renderer::GetSwapChainExt() const
{
	return *_swapChainExt;
}

TextureHandler& Renderer::GetTextureHandler() const
{
	return *_textureHandler;
}

PostEffectHandler& Renderer::GetPostEffectHandler() const
{
	return *_postEffectHandler;
}
