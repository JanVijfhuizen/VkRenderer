#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"

class DescriptorPool;

class SwapChainGC final : public vi::VkHandler
{
public:
	explicit SwapChainGC(vi::VkCore& renderer);
	~SwapChainGC();

	void Update();

	void Enqueue(VkBuffer buffer);
	void Enqueue(VkSampler sampler);
	void Enqueue(VkDeviceMemory memory);
	void Enqueue(VkImage image);
	void Enqueue(VkImageView imageView);
	void Enqueue(VkFramebuffer framebuffer);
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
			framebuffer,
			descriptor
		};

		union
		{
			VkBuffer buffer;
			VkSampler sampler;
			VkDeviceMemory memory;
			VkImage image;
			VkImageView imageView;
			VkFramebuffer framebuffer;
			struct Descriptor final
			{
				VkDescriptorSet set;
				DescriptorPool* pool;
			} descriptor;
		};

		Type type;
		uint32_t index;
	};

	vi::Vector<Deleteable> _deleteables{32, GMEM_VOL};

	void Enqueue(Deleteable& deleteable);
	void Delete(Deleteable& deleteable, bool calledByDetructor);
};
