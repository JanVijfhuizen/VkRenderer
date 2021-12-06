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

	[[nodiscard]] T& Insert(uint32_t sparseId) override;
	void Swap(uint32_t aDenseId, uint32_t bDenseId) override;
	void Erase(uint32_t sparseId) override;

protected:
	virtual void CreateFrame(Frame& frame, uint32_t denseId);
	virtual void EraseFrame(Frame& frame, uint32_t denseId);

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
	auto& t = ce::SparseSet<T>::Insert(sparseId);
	const uint32_t denseId = ce::SparseSet<T>::GetDenseId(sparseId);

	for (uint32_t i = 0; i < _frameCount; ++i)
	{
		auto& frame = _frames[i][denseId];
		frame = {};
		CreateFrame(frame, denseId);
	}

	return t;
}

template <typename T, typename Frame>
void RenderSet<T, Frame>::Swap(const uint32_t aDenseId, const uint32_t bDenseId)
{
	ce::SparseSet<T>::Swap(aDenseId, bDenseId);

	for (uint32_t i = 0; i < _frameCount; ++i)
	{
		auto& aFrame = _frames[i][aDenseId];
		auto& bFrame = _frames[i][bDenseId];

		Frame temp = aFrame;
		aFrame = bFrame;
		bFrame = temp;
	}
}

template <typename T, typename Frame>
void RenderSet<T, Frame>::Erase(const uint32_t sparseId)
{
	const uint32_t denseId = ce::SparseSet<T>::GetDenseId(sparseId);

	for (uint32_t i = 0; i < _frameCount; ++i)
	{
		auto& frame = _frames[i][denseId];
		EraseFrame(frame, denseId);
	}

	ce::SparseSet<T>::Erase(sparseId);
}

template <typename T, typename Frame>
void RenderSet<T, Frame>::CreateFrame(Frame&, const uint32_t)
{
}

template <typename T, typename Frame>
void RenderSet<T, Frame>::EraseFrame(Frame&, const uint32_t)
{
}
