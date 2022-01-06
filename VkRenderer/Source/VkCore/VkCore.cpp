#include "pch.h"
#include "VkCore/VkCore.h"
#include "WindowHandler.h"

namespace vi
{
	VkCore::VkCore(VkCoreInfo& info)
	{
		assert(info.windowHandler);

		// Add required tags.
		info.deviceExtensions.Add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		info.validationLayers.Add("VK_LAYER_KHRONOS_validation");

		// Check debugging support.
		VkCoreDebugger::CheckValidationSupport(info);

		// Set up vulkan instance.
		_instance.Setup(info);
		_debugger.Setup(_instance);

		// Create window surface.
		_windowHandler = info.windowHandler;
		_surface = _windowHandler->CreateSurface(_instance);

		// Set up hardware.
		_physicalDevice.Setup(info, _instance, _surface);

		// Set up hardware interface.
		_logicalDevice.Setup(info, _surface, _physicalDevice);

		// Set up command pool.
		_commandPool.Setup(_surface, _physicalDevice, _logicalDevice);

		// Set up swapchain.
		_swapChain.Construct(_surface, *_windowHandler, _logicalDevice, _physicalDevice);
	}

	VkCore::~VkCore()
	{
		DeviceWaitIdle();

		_swapChain.Cleanup(_logicalDevice);
		_commandPool.Cleanup(_logicalDevice);
		_logicalDevice.Cleanup();
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		_debugger.Cleanup(_instance);
		_instance.Cleanup();
	}

	void VkCore::DeviceWaitIdle() const
	{
		const auto result = vkDeviceWaitIdle(_logicalDevice);
		assert(!result);
	}

	VkSurfaceKHR VkCore::GetSurface() const
	{
		return _surface;
	}

	VkInstance VkCore::GetInstance() const
	{
		return _instance;
	}

	VkPhysicalDevice VkCore::GetPhysicalDevice() const
	{
		return _physicalDevice;
	}

	VkDevice VkCore::GetLogicalDevice() const
	{
		return _logicalDevice;
	}

	VkCommandPool VkCore::GetCommandPool() const
	{
		return _commandPool;
	}
}
