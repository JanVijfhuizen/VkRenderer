#pragma once

namespace vi
{
	class WindowHandlerGLFW;
	class VkRenderer;
	class SwapChain;
}

struct Renderer final
{
	class System final
	{
	public:
		System();
		~System();

		void BeginFrame(bool& quit);
		void EndFrame() const;

		[[nodiscard]] vi::WindowHandlerGLFW& GetWindowHandler() const;
		[[nodiscard]] vi::VkRenderer& GetVkRenderer() const;

	private:
		vi::WindowHandlerGLFW* _windowHandler;
		vi::VkRenderer* _vkRenderer;
		vi::SwapChain* _swapChain;
	};
};

