#pragma once
#include "RenderSystem.h"
#include "VkRenderer/VkRenderer.h"

template <typename T, typename Frame>
class RenderSet : public ce::SparseSet<T>
{
public:
	explicit RenderSet(uint32_t size);
	~RenderSet();

	[[nodiscard]] Frame* GetCurrentFrames();
	[[nodiscard]] Frame** GetAllFrames();

	T& Insert(uint32_t sparseId) override;
	void Swap(uint32_t aDenseId, uint32_t bDenseId) override;

private:
	Frame** _frames;
	uint32_t _frameCount;
};

template <typename T, typename Frame>
RenderSet<T, Frame>::RenderSet(const uint32_t size) : ce::SparseSet<T>(size)
{
	auto& renderSystem = RenderSystem::Get();
	auto& vkRenderer = renderSystem.GetVkRenderer();
	auto& swapChain = vkRenderer.GetSwapChain();

	_frameCount = swapChain.GetImageCount();
	_frames = new Frame*[_frameCount];

	for (uint32_t i = 0; i < _frameCount; ++i)
		_frames[i] = new Frame[size];
}

template <typename T, typename Frame>
RenderSet<T, Frame>::~RenderSet()
{
	for (uint32_t i = 0; i < _frameCount; ++i)
		delete[] _frames[i];
	delete[] _frames;
}

template <typename T, typename Frame>
Frame* RenderSet<T, Frame>::GetCurrentFrames()
{
	auto& renderSystem = RenderSystem::Get();
	auto& vkRenderer = renderSystem.GetVkRenderer();
	auto& swapChain = vkRenderer.GetSwapChain();
	return _frames[swapChain.GetCurrentImageIndex()];
}

template <typename T, typename Frame>
Frame** RenderSet<T, Frame>::GetAllFrames()
{
	return _frames;
}

template <typename T, typename Frame>
T& RenderSet<T, Frame>::Insert(const uint32_t sparseId)
{
	ce::SparseSet<T>::Insert(sparseId);

	// Todo make sure that it works per-frame.
}

template <typename T, typename Frame>
void RenderSet<T, Frame>::Swap(const uint32_t aDenseId, const uint32_t bDenseId)
{
	ce::SparseSet<T>::Swap(aDenseId, bDenseId);

	// Todo make sure that it works per-frame.
}
