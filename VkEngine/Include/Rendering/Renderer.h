#pragma once
#include "VkRenderer/VkCore/VkCore.h"
#include "ShaderExt.h"
#include "SwapChainGC.h"

class Renderer final : public vi::VkCore
{
public:
	explicit Renderer(vi::VkCoreInfo& info);

	[[nodiscard]] ShaderExt& GetShaderExt();
	[[nodiscard]] SwapChainGC& GetSwapChainGC();

private:
	ShaderExt _shaderExt{ *this };
	SwapChainGC _swapChainGC{ *this };
};
