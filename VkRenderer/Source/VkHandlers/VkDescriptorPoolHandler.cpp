#include "pch.h"
#include "VkHandlers/VkDescriptorPoolHandler.h"
#include "VkCore/VkCore.h"
#include "VkHandlers/VkCommandBufferHandler.h"
#include "VkHandlers/VkPipelineHandler.h"

namespace vi
{
	VkDescriptorPool VkDescriptorPoolHandler::Create(const PoolCreateInfo& info) const
	{
		const ArrayPtr<VkDescriptorPoolSize> sizes(info.typeCount, GMEM_TEMP);
		uint32_t maxSets = 0;

		for (uint32_t i = 0; i < info.typeCount; ++i)
		{
			auto& size = sizes[i];
			size.type = info.types[i];
			size.descriptorCount = info.capacities[i];
			maxSets += size.descriptorCount;
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = info.typeCount;
		poolInfo.pPoolSizes = sizes.GetData();
		poolInfo.maxSets = maxSets;

		VkDescriptorPool pool;
		const auto result = vkCreateDescriptorPool(core.GetLogicalDevice(), &poolInfo, nullptr, &pool);
		assert(!result);
		return pool;
	}

	void VkDescriptorPoolHandler::CreateSets(const SetCreateInfo& info) const
	{
		const ArrayPtr<VkDescriptorSetLayout> layouts{ info.setCount, GMEM_TEMP, info.layout };

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = info.pool;
		allocInfo.descriptorSetCount = info.setCount;
		allocInfo.pSetLayouts = layouts.GetData();

		const auto result = vkAllocateDescriptorSets(core.GetLogicalDevice(), &allocInfo, info.outSets);
		assert(!result);
	}

	void VkDescriptorPoolHandler::BindSets(VkDescriptorSet* sets, const uint32_t setCount) const
	{
		vkCmdBindDescriptorSets(core.GetCommandBufferHandler().GetCurrent(), 
			VK_PIPELINE_BIND_POINT_GRAPHICS, core.GetPipelineHandler().GetCurrentLayout(), 
			0, setCount, sets, 0, nullptr);
	}

	void VkDescriptorPoolHandler::Destroy(const VkDescriptorPool pool) const
	{
		vkDestroyDescriptorPool(core.GetLogicalDevice(), pool, nullptr);
	}

	VkDescriptorPoolHandler::VkDescriptorPoolHandler(VkCore& core) : VkHandler(core)
	{

	}
}
