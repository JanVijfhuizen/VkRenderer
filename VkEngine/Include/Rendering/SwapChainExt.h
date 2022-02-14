#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"

class VulkanRenderer;
class DescriptorPool;

/// <summary>
/// Contains engine specific swap chain methods.
/// Also handles swap chain garbage collection.
/// </summary>
class SwapChainExt final : public vi::VkHandler
{
	friend class Dependency;

public:
	/// <summary>
	/// Inherit from this to subscribe to the swap chain recreation event.
	/// Swap chain recreation happens when the window resizes/minimizes,
	/// and it forces some resolution-dependent assets to be recreated.
	/// </summary>
	class Dependency
	{
		friend SwapChainExt;

	public:
		explicit Dependency(VulkanRenderer& renderer);
		virtual ~Dependency();

	protected:
		// Event that fires when the swap chain is being recreated.
		virtual void OnRecreateSwapChainAssets() = 0;

		VulkanRenderer& renderer;
	};

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
		// Type of collectable garbage.
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

		// Use an union to be able to store all the garbage in a single uniform list.
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
				// Reference to the original pool.
				DescriptorPool* pool;
			} descriptor;
		};

		// Garbage type.
		Type type;
		// Swap chain image index from when it was deleted.
		uint32_t index;
		// If the object has made a full swap chain loop.
		bool looped = false;
	};

	// Garbage.
	vi::Vector<Deleteable> _deleteables{32, GMEM_VOL};
	// Recreation event subscribers.
	vi::Vector<Dependency*> _dependencies{ 8, GMEM_VOL };

	void Collect(Deleteable& deleteable);
	void Delete(Deleteable& deleteable, bool calledByDetructor) const;
};
