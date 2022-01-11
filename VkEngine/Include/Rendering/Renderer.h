#pragma once
#include "VkRenderer/VkCore/VkCore.h"
#include "MeshHandler.h"
#include "ShaderExt.h"
#include "SwapChainExt.h"
#include "GPULinearAllocator.h"

class Renderer final : public vi::VkCore
{
public:
	explicit Renderer(vi::VkCoreInfo& info);

	[[nodiscard]] MeshHandler& GetMeshHandler();
	[[nodiscard]] ShaderExt& GetShaderExt();
	[[nodiscard]] SwapChainExt& GetSwapChainExt();
	[[nodiscard]] GPULinearAllocator& GetGPULinearAllocator();

private:
	MeshHandler _meshHandler{ *this };
	ShaderExt _shaderExt{ *this };
	SwapChainExt _swapChainGC{ *this };
	GPULinearAllocator _GPULinearAllocator{ *this };
};
