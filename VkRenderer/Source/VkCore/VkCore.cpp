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
	}

	VkCore::~VkCore()
	{
		DeviceWaitIdle();

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

	void VkCore::LogicalDevice::Setup(
		const VkCoreInfo& info,
		const VkSurfaceKHR surface, 
		const VkCorePhysicalDevice& physicalDevice)
	{
		const auto queueFamilies = VkCorePhysicalDevice::GetQueueFamilies(surface, physicalDevice);

		const uint32_t queueFamiliesCount = sizeof queueFamilies.values / sizeof(uint32_t);
		Vector<VkDeviceQueueCreateInfo> queueCreateInfos{ queueFamiliesCount, GMEM_TEMP};
		HashMap<uint32_t> familyIndexes{queueFamiliesCount, GMEM_TEMP};
		const float queuePriority = 1;

		// Create a queue for each individual queue family.
		// So if a single family handles both graphics and presentation, only create it once.
		// Hence the hashmap.
		for (uint32_t i = 0; i < queueFamiliesCount; ++i)
		{
			const uint32_t family = queueFamilies.values[i];
			if (familyIndexes.Contains(family))
				continue;
			familyIndexes.Insert(family);

			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = family;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.Add(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		const auto& deviceExtensions = info.deviceExtensions;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.GetCount());
		createInfo.pQueueCreateInfos = queueCreateInfos.GetData();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.GetCount());
		createInfo.ppEnabledExtensionNames = deviceExtensions.GetData();

		// Only enable debug layers when in debug mode.
		createInfo.enabledLayerCount = 0;
		#ifdef _DEBUG
		const auto& validationLayers = info.validationLayers;
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.GetCount());
		createInfo.ppEnabledLayerNames = validationLayers.GetData();
		#endif

		const auto result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &value);
		assert(!result);

		uint32_t i = 0;
		for (const auto& family : queueFamilies.values)
		{
			vkGetDeviceQueue(value, family, 0, &queues[i]);
			i++;
		}
	}

	void VkCore::LogicalDevice::Cleanup() const
	{
		vkDestroyDevice(value, nullptr);
	}

	VkCore::LogicalDevice::operator VkDevice() const
	{
		return value;
	}

	void VkCore::CommandPool::Setup(
		const VkSurfaceKHR surface, 
		const VkCorePhysicalDevice& physicalDevice,
		const LogicalDevice& logicalDevice)
	{
		const auto families = VkCorePhysicalDevice::GetQueueFamilies(surface, physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = families.graphics;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		const auto result = vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &value);
		assert(!result);
	}

	void VkCore::CommandPool::Cleanup(const LogicalDevice& logicalDevice) const
	{
		vkDestroyCommandPool(logicalDevice, value, nullptr);
	}

	VkCore::CommandPool::operator VkCommandPool() const
	{
		return value;
	}
}
