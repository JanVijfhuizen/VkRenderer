#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkDescriptorPoolHandler final : public VkHandler
	{
		friend VkCore;

	public:
		[[nodiscard]] VkDescriptorPool Create(const VkDescriptorType* types, const uint32_t* capacities, uint32_t typeCount) const;
		void CreateSets(VkDescriptorPool pool, VkDescriptorSetLayout layout, uint32_t setCount, VkDescriptorSet* outSets) const;
		void BindSets(VkDescriptorSet* sets, uint32_t setCount) const;
		void Destroy(VkDescriptorPool pool) const;

	private:
		explicit VkDescriptorPoolHandler(VkCore& core);
	};
}
