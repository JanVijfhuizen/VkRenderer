#pragma once
#include "UVector.h"

class DescriptorPool final
{
public:
	DescriptorPool();
	DescriptorPool(VkDescriptorSetLayout layout, VkDescriptorType* types, uint32_t* sizes, uint32_t typeCount, uint32_t blockSize);
	DescriptorPool& operator=(DescriptorPool&& other) noexcept;
	~DescriptorPool();

	[[nodiscard]] VkDescriptorSet Get();
	void Add(VkDescriptorSet set);

private:
	VkDescriptorSetLayout _layout;
	VkDescriptorType* _types;
	uint32_t* _sizes;
	uint32_t _typeCount;
	uint32_t _blockSize;

	UVector<VkDescriptorSet> _open;
	UVector<VkDescriptorPool> _subPools{1};

	void AddBlock();
};
