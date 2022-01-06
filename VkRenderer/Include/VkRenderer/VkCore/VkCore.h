#pragma once

#include "VkCoreInfo.h"
#include "VkCoreDebugger.h"
#include "VkCoreInstance.h"
#include "VkCorePhysicalDevice.h"
#include "VkCoreLogicalDevice.h"
#include "VkCoreSwapchain.h"
#include "VkCoreCommandPool.h"

#include "../VkHandlers/VkCommandBufferHandler.h"
#include "../VkHandlers/VkImageHandler.h"

namespace vi
{
	class WindowHandler;

	/// <summary>
	/// Sets up vulkan in a way where it can be used for generic game development purposes.
	/// </summary>
	class VkCore final
	{
	public:
		const VkCommandBufferHandler commandBufferHandler{ *this };
		const VkImageHandler imageHandler{*this};

		explicit VkCore(VkCoreInfo& info);
		~VkCore();

		/// <summary>
		/// Wait until all graphic related tasks are finished.
		/// </summary>
		void DeviceWaitIdle() const;

		[[nodiscard]] VkSurfaceKHR GetSurface() const;
		[[nodiscard]] VkInstance GetInstance() const;
		[[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const;
		[[nodiscard]] VkDevice GetLogicalDevice() const;
		[[nodiscard]] VkCommandPool GetCommandPool() const;
		[[nodiscard]] VkCoreSwapchain& GetSwapChain();

	private:
		WindowHandler* _windowHandler;
		VkSurfaceKHR _surface;

		VkCoreDebugger _debugger;
		VkCoreInstance _instance;
		VkCorePhysicalDevice _physicalDevice;
		VkCoreLogicalDevice _logicalDevice;
		VkCoreCommandPool _commandPool;
		VkCoreSwapchain _swapChain;
	};
}
