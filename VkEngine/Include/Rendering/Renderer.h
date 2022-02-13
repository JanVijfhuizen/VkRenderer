#pragma once
#include "VkRenderer/VkCore/VkCore.h"

class Renderer final : public vi::VkCore
{
public:
	struct Info final
	{
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	};

	explicit Renderer(vi::VkCoreInfo& info, const Info& addInfo);
	~Renderer();

	[[nodiscard]] class MeshHandler& GetMeshHandler() const;
	[[nodiscard]] class ShaderExt& GetShaderExt() const;
	[[nodiscard]] class SwapChainExt& GetSwapChainExt() const;
	[[nodiscard]] class TextureHandler& GetTextureHandler() const;
	[[nodiscard]] class PostEffectHandler& GetPostEffectHandler() const;

private:
	MeshHandler* _meshHandler;
	ShaderExt* _shaderExt;
	TextureHandler* _textureHandler;
	SwapChainExt* _swapChainExt;
	PostEffectHandler* _postEffectHandler;
};
