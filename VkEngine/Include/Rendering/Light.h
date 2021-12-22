#pragma once
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
	VkBuffer _buffer;
	VkDeviceMemory _memory;
	VkDescriptorSet _descriptors[SWAPCHAIN_MAX_FRAMES];
	VkImage _depthImages[SWAPCHAIN_MAX_FRAMES];
	VkDeviceMemory _depthMemories[SWAPCHAIN_MAX_FRAMES];
	VkImageView _depthImageViews[SWAPCHAIN_MAX_FRAMES];
	VkFramebuffer _frameBuffers[SWAPCHAIN_MAX_FRAMES];
};
