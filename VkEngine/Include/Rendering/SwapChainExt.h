#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"

class DescriptorPool;

class SwapChainExt final : public vi::VkHandler
{
public:
	explicit SwapChainExt(vi::VkCore& core);
	~SwapChainExt();

	void Update();

	void Collect(VkBuffer buffer);
	void Collect(VkSampler sampler);
	void Collect(VkDeviceMemory memory);
	void Collect(VkImage image);
	void Collect(VkImageView imageView);
	void Collect(VkFramebuffer framebuffer);
	void Collect(VkDescriptorSet descriptor, DescriptorPool& pool);

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

	void Collect(Deleteable& deleteable);
	void Delete(Deleteable& deleteable, bool calledByDetructor) const;
};
