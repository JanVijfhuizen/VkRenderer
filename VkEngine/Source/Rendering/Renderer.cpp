#include "pch.h"
#include "Rendering/Renderer.h"

Renderer::Renderer(vi::VkCoreInfo& info) : VkCore(info)
{
	_postEffectHandler = GMEM.New<PostEffectHandler>(*this);
}

Renderer::~Renderer()
{
	GMEM.Delete(_postEffectHandler);
}

MeshHandler& Renderer::GetMeshHandler()
{
	return _meshHandler;
}

ShaderExt& Renderer::GetShaderExt()
{
	return _shaderExt;
}

SwapChainExt& Renderer::GetSwapChainExt()
{
	return _swapChainGC;
}

TextureHandler& Renderer::GetTextureHandler()
{
	return _textureHandler;
}

PostEffectHandler& Renderer::GetPostEffectHandler() const
{
	return *_postEffectHandler;
}
