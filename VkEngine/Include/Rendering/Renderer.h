#pragma once
#include "VkRenderer/VkCore/VkCore.h"
#include "ShaderExt.h"

class Renderer final : public vi::VkCore
{
public:
	explicit Renderer(vi::VkCoreInfo& info);

	[[nodiscard]] ShaderExt& GetShaderExt();

private:
	ShaderExt _shaderExt{ *this };
};
