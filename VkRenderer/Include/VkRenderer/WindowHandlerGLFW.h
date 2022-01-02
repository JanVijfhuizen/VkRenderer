#pragma once
#include "WindowHandler.h"

namespace vi
{
	class WindowHandlerGLFW final : public WindowHandler
	{
	public:
		explicit WindowHandlerGLFW(const VkInfo& info = {});
		~WindowHandlerGLFW();

		void BeginFrame(bool& outQuit) const;
		bool QueryHasResized() override;

		const VkInfo& GetVkInfo() const override;

	private:
		GLFWwindow* _window;
		VkInfo _info;

		bool _resized = false;

		[[nodiscard]] VkSurfaceKHR CreateSurface(VkInstance instance) override;
		[[nodiscard]] ArrayPtr<const char*>  GetRequiredExtensions() override;
		static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
	};
}
