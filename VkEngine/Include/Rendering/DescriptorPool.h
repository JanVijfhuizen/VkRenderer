#pragma once
#include "UVector.h"
#include "VkRenderer/ArrayPtr.h"

class DescriptorPool final
{
public:
	DescriptorPool();
	DescriptorPool(VkDescriptorSetLayout layout, ArrayPtr<VkDescriptorType> types, uint32_t blockSize);
	~DescriptorPool();

	[[nodiscard]] VkDescriptorSet Get();
	void Add(VkDescriptorSet set);

private:
	VkDescriptorSetLayout _layout;
	ArrayPtr<VkDescriptorType> _types;
	uint32_t _blockSize;

	UVector<VkDescriptorSet> _open;
	UVector<VkDescriptorPool> _subPools;

	void AddBlock();
};
