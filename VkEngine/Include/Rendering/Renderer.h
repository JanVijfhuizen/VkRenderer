#pragma once
#include "VkRenderer/VkCore/VkCore.h"
#include "ShaderExt.h"
#include "SwapChainGC.h"
#include "MeshHandler.h"

class Renderer final : public vi::VkCore
{
public:
	explicit Renderer(vi::VkCoreInfo& info);

	[[nodiscard]] MeshHandler& GetMeshHandler();
	[[nodiscard]] ShaderExt& GetShaderExt();
	[[nodiscard]] SwapChainGC& GetSwapChainGC();

private:
	MeshHandler _meshHandler{ *this };
	ShaderExt _shaderExt{ *this };
	SwapChainGC _swapChainGC{ *this };
};
