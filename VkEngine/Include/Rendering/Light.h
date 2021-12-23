﻿#pragma once
#include "MapSet.h"
#include "DescriptorPool.h"

struct Light final
{
	class System final : public MapSet<Light>, public Singleton<System>
	{
	public:
		struct Info final
		{
			glm::ivec2 shadowResolution{ 800, 600 };
		};

		explicit System(const Info& info = {});
		~System();

		void Update();

		KeyValuePair<unsigned, Light>& Add(const KeyValuePair<unsigned, Light>& keyPair) override;
		void EraseAt(size_t index) override;

		static void CreateDepthBuffer(glm::ivec2 resolution, VkImage& outImage, VkDeviceMemory& outMemory, VkImageView& outImageView);

	private:
		struct alignas(256) Ubo final
		{
			glm::mat4 lightSpaceMatrix{1};
		};

		Info _info;
		VkShaderModule _vertModule;
		VkDescriptorSetLayout _layout;
		VkRenderPass _renderPass;
		VkPipeline _pipeline;
		VkPipelineLayout _pipelineLayout;
		VkCommandBuffer _commandBuffer;
		VkFence _fence;
		VkDescriptorSetLayout _depthLayout;
		DescriptorPool _descriptorPool;
	};

private:
	struct Frame final
	{
		VkDescriptorSet descriptor;
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory imageMemory;
		VkFramebuffer framebuffer;
	};

	VkBuffer _buffer;
	VkDeviceMemory _memory;
	Frame _frames[SWAPCHAIN_MAX_FRAMES];
};
