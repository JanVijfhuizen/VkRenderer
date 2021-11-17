#pragma once
#include "Debugger.h"
#include "InstanceFactory.h"

namespace vi
{
	class VkRenderer final
	{
	public:
		struct Settings final
		{
			InstanceFactory::Settings instance{};
			Debugger::Settings debugger{};

			class WindowHandler* windowHandler;

			std::vector<const char*> deviceExtensions =
			{
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
		};

		explicit VkRenderer(const Settings& settings);
		~VkRenderer();

		void DeviceWaitIdle() const;

	private:
		class WindowHandler* _windowHandler;
		class Debugger* _debugger = nullptr;

		VkInstance _instance;
		VkSurfaceKHR _surface;
		VkPhysicalDevice _physicalDevice;
	};
}
