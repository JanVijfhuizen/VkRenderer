#include "pch.h"
#include "Rendering/Renderer.h"

Renderer::Renderer(vi::VkCoreInfo& info) : VkCore(info)
{
	
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

GPULinearAllocator& Renderer::GetGPULinearAllocator()
{
	return _GPULinearAllocator;
}
