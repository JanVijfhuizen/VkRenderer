#pragma once
#include "UVector.h"

class SwapChainGC final : public Singleton<SwapChainGC>
{
public:
	~SwapChainGC();

	void Update();

	void Enqueue(VkBuffer buffer);
	void Enqueue(VkSampler sampler);
	void Enqueue(VkDeviceMemory memory);

private:
	struct Deleteable final
	{
		enum class Type
		{
			buffer,
			sampler,
			memory
		};

		union
		{
			VkBuffer buffer;
			VkSampler sampler;
			VkDeviceMemory memory;
		};

		Type type;
		uint32_t index;
	};

	UVector<Deleteable> _deleteables;

	void Enqueue(Deleteable& deleteable);
	void Delete(Deleteable& deleteable);
};
