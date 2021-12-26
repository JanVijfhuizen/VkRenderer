#pragma once
#include "MapSet.h"
#include "DescriptorPool.h"

struct Light final
{
	enum class Type
	{
		directional,
		point
	};

	union
	{
		struct
		{
			
		} directional;
		struct
		{
			
		} point;
	};

	Type type = Type::directional;

	class System final : public MapSet<Light>, public Singleton<System>
	{
	public:
		struct Info final
		{
			glm::ivec2 shadowResolution{ 800, 600 };
			float near = 0.1f;
			float far = 1000;
		};

		explicit System(const Info& info = {});
		~System();

		void Update();

		KeyValuePair<unsigned, Light>& Add(const KeyValuePair<unsigned, Light>& keyPair) override;
		void EraseAt(size_t index) override;

		[[nodiscard]] VkDescriptorSetLayout GetLayout() const;
		static void CreateDepthBuffer(glm::ivec2 resolution, VkImage& outImage, VkDeviceMemory& outMemory, VkImageView& outImageView);

	private:
		struct alignas(256) Ubo final
		{
			glm::mat4 lightSpaceMatrix{1};
		};

		Info _info;
		VkShaderModule _vertModule;
		VkDescriptorSetLayout _layout;
		VkDescriptorSetLayout _layoutExt;
		VkRenderPass _renderPass;
		VkPipeline _pipeline;
		VkPipelineLayout _pipelineLayout;
		VkCommandBuffer _commandBuffer;
		VkFence _fence;
		VkDescriptorSetLayout _depthLayout;
		DescriptorPool _descriptorPool;
		DescriptorPool _descriptorPoolExt;
	};

	[[nodiscard]] const VkDescriptorSet* GetDescriptors() const;

private:
	struct Frame final
	{
		VkDescriptorSet descriptor;
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory imageMemory;
		VkFramebuffer framebuffer;

		VkSampler samplerExt;
	};

	VkBuffer _buffer;
	VkDeviceMemory _memory;
	Frame _frames[SWAPCHAIN_MAX_FRAMES];
	VkDescriptorSet _descriptorsExt[SWAPCHAIN_MAX_FRAMES];
};
