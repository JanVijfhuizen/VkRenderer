#pragma once
#include "Queues.h"

namespace vi
{
	class LogicalDeviceFactory final
	{
	public:
		struct Info final
		{
			VkSurfaceKHR surface;
			VkPhysicalDevice physicalDevice;
			std::vector<const char*> deviceExtensions;
			std::vector<const char*> validationLayers;

			class Debugger* debugger;
		};

		struct Out final
		{
			VkDevice device;
			Queues queues;
		};

		[[nodiscard]] static Out Construct(const Info& info);
	};
}
