#pragma once
#include "VkRenderer/StackAllocator.h"
#include "SwapChainGC.h"

namespace vi
{
	class WindowHandlerGLFW;
	class VkRenderer;
	class SwapChain;
}

class RenderSystem final : public Singleton<RenderSystem>
{
public:
	RenderSystem();
	~RenderSystem();

	void BeginFrame(bool& quit);
	void EndFrame();

	[[nodiscard]] vi::WindowHandlerGLFW& GetWindowHandler() const;
	[[nodiscard]] vi::VkRenderer& GetVkRenderer() const;

private:
	vi::StackAllocator _allocator{};
	vi::WindowHandlerGLFW* _windowHandler;
	vi::VkRenderer* _vkRenderer;
	vi::SwapChain* _swapChain;
	SwapChainGC _swapChainGBCollector{};
};