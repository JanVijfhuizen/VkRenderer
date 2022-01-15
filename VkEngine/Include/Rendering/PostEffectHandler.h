#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"
#include "SwapChainExt.h"

class Renderer;

class PostEffectHandler final : public vi::VkHandler, public SwapChainExt::Dependency
{
public:
	explicit PostEffectHandler(Renderer& renderer);
	~PostEffectHandler();

	[[nodiscard]] VkRenderPass GetRenderPass() const;
	[[nodiscard]] VkExtent2D GetExtent() const;

protected:
	void OnRecreateSwapChainAssets() override;

private:
	Renderer& _renderer;
	VkRenderPass _renderPass = VK_NULL_HANDLE;
	VkExtent2D _extent;

	void DestroySwapChainAssets() const;
};
