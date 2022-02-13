#pragma once

class Renderer;

/// <summary>
/// A descriptor pool container that automatically resizes when necessary. 
/// Uses volatile memory, so access will be slower.
/// </summary>
class DescriptorPool final
{
public:
	/// <param name="layout">Descriptor set layout.</param>
	/// <param name="types">Descriptor types.</param>
	/// <param name="capacities">Capacities for the descriptor types.</param>
	/// <param name="typeCount">Amount of types.</param>
	/// <param name="blockSize">Descriptor set capacity per descriptor pool.</param>
	void Construct(Renderer& renderer, VkDescriptorSetLayout layout,
		VkDescriptorType* types, uint32_t* capacities, uint32_t typeCount,
	    uint32_t blockSize);
	void Cleanup();

	// Get a new descriptor set. If none are available, allocate a new descriptor pool and get it from there.
	[[nodiscard]] VkDescriptorSet Get();
	// Add a descriptor set back into the pool.
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
