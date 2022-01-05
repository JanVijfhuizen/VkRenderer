#pragma once

#include "VkCoreInfo.h"
#include "VkCoreDebugger.h"
#include "VkCoreInstance.h"
#include "VkCorePhysicalDevice.h"
#include "VkCoreSwapchain.h"

namespace vi
{
	class WindowHandler;

	/// <summary>
	/// Sets up vulkan in a way where it can be used for generic game development purposes.
	/// </summary>
	class VkCore final
	{
	public:
		explicit VkCore(VkCoreInfo& info);
		~VkCore();

		void DeviceWaitIdle() const;

	private:
		WindowHandler* _windowHandler;
		VkSurfaceKHR _surface;

		VkCoreDebugger _debugger;
		VkCoreInstance _instance;
		VkCorePhysicalDevice _physicalDevice;

		struct LogicalDevice final 
		{
			VkDevice value;

			union
			{
				struct
				{
					VkQueue graphics;
					VkQueue present;
				};
				VkQueue queues[2];
			};

			void Setup(const VkCoreInfo& info, VkSurfaceKHR surface, const VkCorePhysicalDevice& physicalDevice);
			void Cleanup() const;
			operator VkDevice() const;
		} _logicalDevice;

		struct CommandPool final
		{
			VkCommandPool value;

			void Setup(VkSurfaceKHR surface, const VkCorePhysicalDevice& physicalDevice, const LogicalDevice& logicalDevice);
			void Cleanup(const LogicalDevice& logicalDevice) const;
			operator VkCommandPool() const;
		} _commandPool;

		VkCoreSwapchain _swapChain;
	};
}
