#pragma once
#include "VkRenderer/VkCore/VkCore.h"

/// <summary>
/// Handy class that extends VkCore and manages some core rendering classes.
/// </summary>
class Renderer final : public vi::VkCore
{
public:
	// Used to create the renderer with.
	struct Info final
	{
		// Anti aliasing sample count. Will be lowered if the hardware doesn't support it.
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
