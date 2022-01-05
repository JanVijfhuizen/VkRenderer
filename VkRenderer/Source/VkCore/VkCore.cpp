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

	VkCore::PhysicalDevice::QueueFamilies::operator bool() const
	{
		for (const auto& family : values)
			if (family == UINT32_MAX)
				return false;
		return true;
	}

	void VkCore::PhysicalDevice::Setup(
		const VkCoreInfo& info,
		const VkCoreInstance& instance, 
		const VkSurfaceKHR& surface)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		assert(deviceCount);

		ArrayPtr<VkPhysicalDevice> devices{deviceCount, GMEM_TEMP};
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.GetData());

		BinTree<VkPhysicalDevice> candidates{ deviceCount, GMEM_TEMP};

		// Add all potential candidates in a sorted way.
		for (const auto& device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			deviceFeatures.samplerAnisotropy = VK_TRUE;

			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			const auto families = GetQueueFamilies(surface, device);
			if (!families)
				continue;

			if (!CheckDeviceExtensionSupport(device, info.deviceExtensions))
				continue;

			const DeviceInfo deviceInfo
			{
				device,
				deviceProperties,
				deviceFeatures
			};

			if (!IsDeviceSuitable(surface, deviceInfo))
				continue;

			const int32_t score = RateDevice(deviceInfo);
			candidates.Push({ score, device });
		}

		assert(!candidates.IsEmpty());
		value = candidates.Peek();
	}

	VkCore::PhysicalDevice::operator VkPhysicalDevice() const
	{
		return value;
	}

	VkCore::PhysicalDevice::QueueFamilies VkCore::PhysicalDevice::GetQueueFamilies(
		const VkSurfaceKHR surface,
		const VkPhysicalDevice physicalDevice)
	{
		QueueFamilies families{};

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		ArrayPtr<VkQueueFamilyProperties> queueFamilies{queueFamilyCount, GMEM_TEMP};
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.GetData());

		// Check for hardware capabilities.
		uint32_t i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				families.graphics = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			if (presentSupport)
				families.present = i;

			if (families)
				break;
			i++;
		}

		return families;
	}

	bool VkCore::PhysicalDevice::IsDeviceSuitable(const VkSurfaceKHR surface, const DeviceInfo& deviceInfo)
	{
		const auto swapChainSupport = SwapChain::QuerySwapChainSupport(surface, deviceInfo.device);	
		if (!swapChainSupport)
			return false;
		if (!deviceInfo.features.samplerAnisotropy)
			return false;
		return true;
	}

	uint32_t VkCore::PhysicalDevice::RateDevice(const DeviceInfo& deviceInfo)
	{
		uint32_t score = 0;
		auto& properties = deviceInfo.properties;

		// Arbitrary increase in score, not sure what to look for to be honest.
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000;
		score += properties.limits.maxImageDimension2D;

		return score;
	}

	bool VkCore::PhysicalDevice::CheckDeviceExtensionSupport(
		const VkPhysicalDevice device,
		const ArrayPtr<const char*>& extensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		ArrayPtr<VkExtensionProperties> availableExtensions{ extensionCount, GMEM_TEMP };
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.GetData());

		// Check if every required extension is available.
		HashMap<CStrRef> hashMap{ extensionCount, GMEM_TEMP };
		for (auto& extension : extensions)
			hashMap.Insert(extension);

		for (const auto& extension : availableExtensions)
			hashMap.Erase(extension.extensionName);

		return hashMap.IsEmpty();
	}

	void VkCore::LogicalDevice::Setup(
		const VkCoreInfo& info,
		const VkSurfaceKHR surface, 
		const PhysicalDevice& physicalDevice)
	{
		const auto queueFamilies = PhysicalDevice::GetQueueFamilies(surface, physicalDevice);

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
		const PhysicalDevice& physicalDevice, 
		const LogicalDevice& logicalDevice)
	{
		const auto families = PhysicalDevice::GetQueueFamilies(surface, physicalDevice);

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

	VkCore::SwapChain::SupportDetails::operator bool() const
	{
		return !formats.IsNull() && !presentModes.IsNull();
	}

	uint32_t VkCore::SwapChain::SupportDetails::GetRecommendedImageCount() const
	{
		uint32_t imageCount = capabilities.minImageCount + 1;

		const auto& maxImageCount = capabilities.maxImageCount;
		if (maxImageCount > 0 && imageCount > maxImageCount)
			imageCount = maxImageCount;

		if (imageCount > SWAPCHAIN_MAX_FRAMES)
			imageCount = SWAPCHAIN_MAX_FRAMES;
		return imageCount;
	}

	VkCore::SwapChain::SupportDetails VkCore::SwapChain::QuerySwapChainSupport(
		const VkSurfaceKHR surface,
		const VkPhysicalDevice device)
	{
		SupportDetails details{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
		
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			auto& formats = details.formats;
			formats = ArrayPtr<VkSurfaceFormatKHR>(formatCount, GMEM_TEMP);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.GetData());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			auto& presentModes = details.presentModes;
			presentModes = ArrayPtr<VkPresentModeKHR>(presentModeCount, GMEM_TEMP);

			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
				&presentModeCount, details.presentModes.GetData());
		}

		return details;
	}
}
