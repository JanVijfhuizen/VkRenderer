#pragma once
#include "WindowHandler.h"

namespace vi
{
	/// <summary>
	/// Handles the windowing of the application. Can open and close windows.
	/// </summary>
	class WindowHandlerGLFW final : public WindowHandler
	{
	public:
		explicit WindowHandlerGLFW(const Info& info = {});
		~WindowHandlerGLFW();

		/// <summary>Call this at the start of the frame.</summary>
		void BeginFrame(bool& outQuit) const;
		bool QueryHasResized() override;

	private:
		GLFWwindow* _window;

		bool _resized = false;

		/// <summary>Creates a Vulkan surface.</summary>
		[[nodiscard]] VkSurfaceKHR CreateSurface(VkInstance instance) override;
		[[nodiscard]] ArrayPtr<const char*>  GetRequiredExtensions() override;
		static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
	};
}
