#include "pch.h"
#include "Rendering/Renderer.h"

Renderer::Renderer(vi::VkCoreInfo& info) : VkCore(info)
{
	
}

ShaderExt& Renderer::GetShaderExt()
{
	return _shaderExt;
}

SwapChainGC& Renderer::GetSwapChainGC()
{
	return _swapChainGC;
}
