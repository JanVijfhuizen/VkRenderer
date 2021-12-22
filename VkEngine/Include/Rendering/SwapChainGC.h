#pragma once
#include "UVector.h"

class DescriptorPool;

class SwapChainGC final : public Singleton<SwapChainGC>
{
public:
	~SwapChainGC();

	void Update();

	void Enqueue(VkBuffer buffer);
	void Enqueue(VkSampler sampler);
	void Enqueue(VkDeviceMemory memory);
	void Enqueue(VkImage image);
	void Enqueue(VkImageView imageView);
	void Enqueue(VkDescriptorSet descriptor, DescriptorPool& pool);

private:
	struct Deleteable final
	{
		enum class Type
		{
			buffer,
			sampler,
			memory,
			image,
			imageView,
			descriptor
		};

		union
		{
			VkBuffer buffer;
			VkSampler sampler;
			VkDeviceMemory memory;
			VkImage image;
			VkImageView imageView;
			struct Descriptor final
			{
				VkDescriptorSet set;
				DescriptorPool* pool;
			} descriptor;
		};

		Type type;
		uint32_t index;
	};

	UVector<Deleteable> _deleteables;

	void Enqueue(Deleteable& deleteable);
	void Delete(Deleteable& deleteable, bool calledByDetructor);
};
