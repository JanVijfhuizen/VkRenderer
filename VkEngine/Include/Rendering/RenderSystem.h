#pragma once
#include "VkRenderer/FreeListAllocator.h"

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
	vi::FreeListAllocator _allocator{2e3f};
	vi::WindowHandlerGLFW* _windowHandler;
	vi::VkRenderer* _vkRenderer;
	vi::SwapChain* _swapChain;
};