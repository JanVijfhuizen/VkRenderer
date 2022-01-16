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
	struct Info final
	{
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	};

	explicit Renderer(vi::VkCoreInfo& info, const Info& addInfo);
	~Renderer();

	[[nodiscard]] MeshHandler& GetMeshHandler() const;
	[[nodiscard]] ShaderExt& GetShaderExt() const;
	[[nodiscard]] SwapChainExt& GetSwapChainExt() const;
	[[nodiscard]] TextureHandler& GetTextureHandler() const;
	[[nodiscard]] PostEffectHandler& GetPostEffectHandler() const;

private:
	MeshHandler* _meshHandler;
	ShaderExt* _shaderExt;
	TextureHandler* _textureHandler;
	SwapChainExt* _swapChainExt;
	PostEffectHandler* _postEffectHandler;
};
