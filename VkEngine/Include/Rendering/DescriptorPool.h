#pragma once

class Renderer;

class DescriptorPool final
{
public:
	DescriptorPool(Renderer& renderer, VkDescriptorSetLayout layout,
		vi::ArrayPtr<VkDescriptorType>& types, vi::ArrayPtr<uint32_t>& capacities,
	    uint32_t blockSize);
	~DescriptorPool();

	[[nodiscard]] VkDescriptorSet Get();
	void Add(VkDescriptorSet set);

private:
	Renderer& _renderer;
	VkDescriptorSetLayout _layout;
	vi::ArrayPtr<VkDescriptorType> _types;
	vi::ArrayPtr<uint32_t> _capacities;
	uint32_t _blockSize;

	vi::Vector<VkDescriptorPool> _pools{1, GMEM_VOL};
	vi::Vector<VkDescriptorSet> _open{0, GMEM_VOL};

	void AddBlock();
};
