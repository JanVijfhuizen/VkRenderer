﻿#pragma once
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

	private:
		GLFWwindow* _window;
		VkInfo _info;

		bool _resized = false;

		[[nodiscard]] VkSurfaceKHR CreateSurface(VkInstance instance) override;
		const VkInfo& GetVkInfo() const override;
		void GetRequiredExtensions(std::vector<const char*>& extensions) override;

		static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
	};
}