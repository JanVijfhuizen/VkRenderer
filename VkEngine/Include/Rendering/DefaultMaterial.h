#pragma once
#include "DescriptorPool.h"

struct DefaultMaterial final
{
	uint32_t textureHandle = 0;

	class System final : public ce::SparseSet<DefaultMaterial>, public Singleton<System>
	{
	public:
		explicit System(uint32_t size);
		~System();

		void Update();

		DefaultMaterial& Insert(uint32_t sparseId) override;
		void Erase(uint32_t sparseId) override;

	private:
		VkShaderModule _vertModule;
		VkShaderModule _fragModule;
		VkDescriptorSetLayout _layout;
		VkPipeline _pipeline;
		VkPipelineLayout _pipelineLayout;
		DescriptorPool _descriptorPool;

		void OnErase(uint32_t sparseId);
	};

private:
	VkSampler _diffuseSamplers[SWAPCHAIN_MAX_FRAMES];
	VkDescriptorSet _descriptors[SWAPCHAIN_MAX_FRAMES];
};
