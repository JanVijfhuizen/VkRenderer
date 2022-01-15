﻿#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"
#include "SwapChainExt.h"
#include "ShaderExt.h"
#include "DescriptorPool.h"
#include "MeshHandler.h"

class Renderer;

class PostEffectHandler final : public vi::VkHandler, public SwapChainExt::Dependency
{
public:
	explicit PostEffectHandler(Renderer& renderer);
	~PostEffectHandler();

	void BeginFrame();
	void EndFrame();

	void Draw();

	[[nodiscard]] VkRenderPass GetRenderPass() const;
	[[nodiscard]] VkExtent2D GetExtent() const;

protected:
	void OnRecreateSwapChainAssets() override;

private:
	struct Frame final
	{
		VkImage colorImage;
		VkImage depthImage;
		VkDeviceMemory colorMemory;
		VkDeviceMemory depthMemory;
		VkFramebuffer frameBuffer;
		VkCommandBuffer commandBuffer;
		VkFence fence;

		union
		{
			struct
			{
				VkImageView imageView;
				VkImageView depthImageView;
			};
			VkImageView imageViews[2]
			{
				VK_NULL_HANDLE,
				VK_NULL_HANDLE
			};
		};

		VkDescriptorSet descriptorSet;
	};

	Renderer& _renderer;
	VkRenderPass _renderPass = VK_NULL_HANDLE;
	VkExtent2D _extent;

	Frame _frames[SWAPCHAIN_MAX_FRAMES];
	uint32_t _imageIndex;

	Shader _shader;
	VkDescriptorSetLayout _layout;
	Mesh _mesh;
	DescriptorPool _descriptorPool{};

	VkPipeline _pipeline;
	VkPipelineLayout _pipelineLayout;

	void DestroySwapChainAssets() const;
};
