#pragma once
#include "UVector.h"

class SwapChainGC final : public Singleton<SwapChainGC>
{
public:
	~SwapChainGC();

	void Update();

	void Enqueue(VkBuffer buffer);
	void Enqueue(VkSampler sampler);

private:
	struct Deleteable final
	{
		union
		{
			VkBuffer buffer;
			VkSampler sampler;
		};

		uint32_t type;
		uint32_t index;
	};

	UVector<Deleteable> _deleteables;

	void Enqueue(Deleteable& deleteable);
	void Delete(Deleteable& deleteable);
};
