#pragma once

class SwapChainGC final : public Singleton<SwapChainGC>
{
public:
	void Update();
	void FreeAll();

	void Collect(VkBuffer buffer, uint32_t imageIndex);
	void Collect(VkSampler sampler, uint32_t imageIndex);

private:
	struct ImageQueue final
	{
		std::vector<VkBuffer> buffers;
		std::vector<VkSampler> samplers;
	};

	std::unordered_map<uint32_t, ImageQueue> _queues{};
	uint32_t _imageIndex = 0;

	static void Free(ImageQueue& queue);
};
