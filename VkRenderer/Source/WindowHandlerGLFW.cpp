#include "pch.h"
#include "WindowHandlerGLFW.h"

namespace vi
{
	WindowHandlerGLFW::WindowHandlerGLFW(const VkInfo& info) : _info(info)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		const auto& resolution = info.resolution;
		_window = glfwCreateWindow(resolution.x, resolution.y, info.name.c_str(), nullptr, nullptr);
		assert(_window);
		glfwSetWindowUserPointer(_window, this);
		glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);
	}

	WindowHandlerGLFW::~WindowHandlerGLFW()
	{
		glfwDestroyWindow(_window);
		glfwTerminate();
	}

	void WindowHandlerGLFW::BeginFrame(bool& outQuit) const
	{
		outQuit = glfwWindowShouldClose(_window);
		if (outQuit)
			return;

		glfwPollEvents();

		int32_t width = 0, height = 0;
		glfwGetFramebufferSize(_window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(_window, &width, &height);
			glfwWaitEvents();
		}
	}

	void WindowHandlerGLFW::CreateSurface(const VkInstance instance, VkSurfaceKHR& surface)
	{
		const auto result = glfwCreateWindowSurface(instance, _window, nullptr, &surface);
		assert(!result);
	}

	const WindowHandler::VkInfo& WindowHandlerGLFW::GetVkInfo() const
	{
		return _info;
	}

	bool WindowHandlerGLFW::QueryHasResized()
	{
		const bool resized = _resized;
		_resized = false;
		return resized;
	}

	void WindowHandlerGLFW::GetRequiredExtensions(std::vector<const char*>& extensions)
	{
		uint32_t glfwExtensionCount = 0;
		const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (uint32_t i = 0; i < glfwExtensionCount; ++i)
			extensions.push_back(glfwExtensions[i]);
	}

	void WindowHandlerGLFW::FramebufferResizeCallback(GLFWwindow* window, const int width, const int height)
	{
		auto self = reinterpret_cast<WindowHandlerGLFW*>(glfwGetWindowUserPointer(window));
		self->_resized = true;
		self->_info.resolution = { width, height };
	}
}