#pragma once
#include "VkHandler.h"

namespace vi
{
	/// <summary>
	/// Contains some core functionality for vulkan descriptor pools.
	/// </summary>
	class VkDescriptorPoolHandler final : public VkHandler
	{
	public:
		/// <summary>
		/// Struct that contains information when creating a descriptor pool.
		/// </summary>
		struct PoolCreateInfo final
		{
			// All the types of buffers the sets can have.
			VkDescriptorType* types;
			// The amount of each type of buffer present in the pool.
			const uint32_t* capacities;
			uint32_t typeCount;
		};

		/// <summary>
		/// Struct that contains information when submitting a descriptor set.
		/// </summary>
		struct SetCreateInfo final
		{
			// Pool to be allocated from.
			VkDescriptorPool pool;
			// Descriptor layout which to replicate.
			VkDescriptorSetLayout layout;
			uint32_t setCount;
			// Parses allocated sets in here.
			VkDescriptorSet* outSets;
		};

		explicit VkDescriptorPoolHandler(VkCore& core);

		/// <returns>Object that can allocate new descriptor sets.</returns>
		[[nodiscard]] VkDescriptorPool Create(const PoolCreateInfo& info) const;
		void CreateSets(const SetCreateInfo& info) const;
		/// <summary>
		/// Binds any number of sets (soft cap on most hardware is 4) to be used to forward data to the GPU.
		/// </summary>
		/// <param name="sets">Sets which to bind.</param>
		void BindSets(VkDescriptorSet* sets, uint32_t setCount) const;
		void Destroy(VkDescriptorPool pool) const;
	};
}
