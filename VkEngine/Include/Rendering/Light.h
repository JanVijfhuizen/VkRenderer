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
		
	private:
		struct Ubo final
		{
			
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
};
