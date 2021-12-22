#pragma once

namespace vi
{
	class WindowHandlerGLFW;
	class VkRenderer;
	class SwapChain;
}

class RenderManager final : public Singleton<RenderManager>
{
public:
	RenderManager();
	~RenderManager();

	void BeginFrame(bool& quit);
	void EndFrame();

	[[nodiscard]] vi::WindowHandlerGLFW& GetWindowHandler() const;
	[[nodiscard]] vi::VkRenderer& GetVkRenderer() const;

private:
	vi::WindowHandlerGLFW* _windowHandler;
	vi::VkRenderer* _vkRenderer;
	vi::SwapChain* _swapChain;
};