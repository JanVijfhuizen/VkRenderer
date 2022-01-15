#pragma once
#include "VkRenderer/VkCore/VkCore.h"
#include "MeshHandler.h"
#include "ShaderExt.h"
#include "SwapChainExt.h"
#include "TextureHandler.h"
#include "PostEffectHandler.h"

class Renderer final : public vi::VkCore
{
public:
	explicit Renderer(vi::VkCoreInfo& info);

	[[nodiscard]] MeshHandler& GetMeshHandler();
	[[nodiscard]] ShaderExt& GetShaderExt();
	[[nodiscard]] SwapChainExt& GetSwapChainExt();
	[[nodiscard]] TextureHandler& GetTextureHandler();
	[[nodiscard]] PostEffectHandler& GetPostEffectHandler();

private:
	MeshHandler _meshHandler{ *this };
	ShaderExt _shaderExt{ *this };
	TextureHandler _textureHandler{ *this };
	SwapChainExt _swapChainGC{ *this };
	PostEffectHandler _postEffectHandler{ *this };
};
