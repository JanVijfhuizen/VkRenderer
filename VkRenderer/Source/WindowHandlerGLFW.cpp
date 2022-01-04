#include "pch.h"
#include "WindowHandlerGLFW.h"

namespace vi
{
	WindowHandlerGLFW::WindowHandlerGLFW(const Info& info) : WindowHandler(info)
	{
		// Initialize GLFW for Vulkan.
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Create window.
		const auto& resolution = info.resolution;
		_window = glfwCreateWindow(resolution.x, resolution.y, info.name.GetData(), nullptr, nullptr);
		assert(_window);
		glfwSetWindowUserPointer(_window, this);

		// Set callback for resize.
		glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);
	}

	WindowHandlerGLFW::~WindowHandlerGLFW()
	{
		glfwDestroyWindow(_window);
		glfwTerminate();
	}

	void WindowHandlerGLFW::BeginFrame(bool& outQuit) const
	{
		// Check if the user pressed the close button.
		outQuit = glfwWindowShouldClose(_window);
		if (outQuit)
			return;

		// Check for events.
		glfwPollEvents();

		int32_t width = 0, height = 0;
		glfwGetFramebufferSize(_window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(_window, &width, &height);
			glfwWaitEvents();
		}
	}

	VkSurfaceKHR WindowHandlerGLFW::CreateSurface(const VkInstance instance)
	{
		VkSurfaceKHR surface;
		const auto result = glfwCreateWindowSurface(instance, _window, nullptr, &surface);
		assert(!result);
		return surface;
	}

	ArrayPtr<const char*> WindowHandlerGLFW::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const auto buffer = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// Copy data from buffer into the array.
		ArrayPtr<const char*> extensions(glfwExtensionCount, GMEM_TEMP);
		memcpy(extensions.GetData(), buffer, sizeof(const char*) * glfwExtensionCount);
		return extensions;
	}

	bool WindowHandlerGLFW::QueryHasResized()
	{
		const bool resized = _resized;
		_resized = false;
		return resized;
	}

	void WindowHandlerGLFW::FramebufferResizeCallback(GLFWwindow* window, const int width, const int height)
	{
		auto self = reinterpret_cast<WindowHandlerGLFW*>(glfwGetWindowUserPointer(window));
		self->_resized = true;
		self->info.resolution = { width, height };
	}
}