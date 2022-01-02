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
			float far = 50;
			uint32_t maxLights = 8;
		};

		struct DepthBuffer final
		{
			VkImage image;
			VkImageView imageView;
			VkDeviceMemory imageMemory;
		};

		explicit System(const Info& info = {});
		~System();

		void Update();

		KeyValuePair<unsigned, Light>& Add(const KeyValuePair<unsigned, Light>& keyPair) override;
		void EraseAt(size_t index) override;

		[[nodiscard]] VkDescriptorSetLayout GetLayout() const;
		[[nodiscard]] VkDescriptorSet GetDescriptor() const;

		static void CreateDepthBuffer(glm::ivec2 resolution, DepthBuffer& outDepthBuffer);
		static void DestroyDepthBuffer(DepthBuffer& depthBuffer);

	private:
		struct alignas(256) UboLight final
		{
			glm::mat4 lightSpaceMatrix{1};
			glm::vec3 lightDir{0, 0, 1};
		};

		struct alignas(256) UboLightInfo final
		{
			uint32_t count;
		};

		struct Frame final
		{
			VkDescriptorSet descriptor;
		};

		struct Instance final
		{
			VkSampler sampler;
			VkFramebuffer frameBuffer;
			DepthBuffer depthBuffer;
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

		VkBuffer _uboBuffer;
		VkDeviceMemory _uboMemory;
		Instance* _instances;
		Frame* _frames;
	};

private:
	struct Frame final
	{
		VkDescriptorSet descriptor;
	};

	Frame _frames[SWAPCHAIN_MAX_FRAMES];
};
