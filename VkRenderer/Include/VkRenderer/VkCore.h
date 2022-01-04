#pragma once

namespace vi
{
	class WindowHandler;

	class VkCore final
	{
	public:
		struct Info final
		{
			//InstanceFactory::Settings instance{};
			//Debugger::Settings debugger{};

			WindowHandler* windowHandler;

			Vector<const char*> deviceExtensions; // VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
	};
}
