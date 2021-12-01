#pragma once

namespace vi
{
	class WindowHandlerGLFW;
	class VkRenderer;
	class SwapChain;
}

class RenderSystem final
{
public:
	RenderSystem();
	~RenderSystem();

	void BeginFrame(bool& quit);
	void EndFrame();

	vi::WindowHandlerGLFW& GetWindowHandler() const;
	vi::VkRenderer& GetVkRenderer() const;

private:
	vi::WindowHandlerGLFW* _windowHandler;
	vi::VkRenderer* _vkRenderer;
	vi::SwapChain* _swapChain;
};
