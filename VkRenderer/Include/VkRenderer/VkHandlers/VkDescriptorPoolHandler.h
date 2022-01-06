#pragma once
#include "VkHandler.h"

namespace vi
{
	/// <summary>
	/// Contains all the descriptor related methods for the renderer.
	/// </summary>
	class VkDescriptorPoolHandler final : public VkHandler
	{
		friend VkCore;

	public:
		/// <param name="types">All the types of buffers the sets can have.</param>
		/// <param name="capacities">The amount of each type of buffer present in the pool.</param>
		/// <returns>Object that can allocate new descriptor sets.</returns>
		[[nodiscard]] VkDescriptorPool Create(const VkDescriptorType* types, const uint32_t* capacities, uint32_t typeCount) const;
		/// <param name="pool">Pool to be allocated from.</param>
		/// <param name="layout">Descriptor layout which to replicate.</param>
		/// <param name="outSets">Parses allocated sets in here.</param>
		void CreateSets(VkDescriptorPool pool, VkDescriptorSetLayout layout, uint32_t setCount, VkDescriptorSet* outSets) const;
		/// <summary>
		/// Binds any number of sets (soft cap on most hardware is 4) to be used to forward data to the GPU.
		/// </summary>
		/// <param name="sets">Sets which to bind.</param>
		void BindSets(VkDescriptorSet* sets, uint32_t setCount) const;
		void Destroy(VkDescriptorPool pool) const;

	private:
		explicit VkDescriptorPoolHandler(VkCore& core);
	};
}
