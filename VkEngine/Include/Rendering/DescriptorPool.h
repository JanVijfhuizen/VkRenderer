#pragma once

class Renderer;

class DescriptorPool final
{
public:
	void Construct(Renderer& renderer, VkDescriptorSetLayout layout,
		VkDescriptorType* types, uint32_t* capacities, uint32_t typeCount,
	    uint32_t blockSize);
	void Cleanup();

	[[nodiscard]] VkDescriptorSet Get();
	void Add(VkDescriptorSet set);

private:
	Renderer* _renderer;
	VkDescriptorSetLayout _layout;
	vi::ArrayPtr<VkDescriptorType> _types;
	vi::ArrayPtr<uint32_t> _capacities;
	uint32_t _blockSize;

	vi::Vector<VkDescriptorPool> _pools{0, GMEM_VOL};
	vi::Vector<VkDescriptorSet> _open{0, GMEM_VOL};

	void AddBlock();
};
