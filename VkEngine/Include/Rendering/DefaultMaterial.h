#pragma once
#include "DescriptorPool.h"

struct DefaultMaterial final
{
	class System final : public ce::SparseSet<DefaultMaterial>, public Singleton<System>
	{
	public:
		explicit System(uint32_t size);
		~System();

		void Update();

	private:
		VkShaderModule _vertModule;
		VkShaderModule _fragModule;
		VkDescriptorSetLayout _layout;
		VkPipeline _pipeline;
		VkPipelineLayout _pipelineLayout;
		DescriptorPool _descriptorPool;
	};
};
