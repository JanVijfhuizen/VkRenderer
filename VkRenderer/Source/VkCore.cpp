#include "pch.h"
#include "VkCore.h"
#include "WindowHandler.h"

namespace vi
{
	VkCore::VkCore(Info& info)
	{
		assert(info.windowHandler);

		// Add required tags.
		info.deviceExtensions.Add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		info.validationLayers.Add("VK_LAYER_KHRONOS_validation");

		// Check debugging support.
		_debugger.CheckValidationSupport(info);

		// Set up vulkan instance.
		_instance.Setup(info, _debugger);
		_debugger.Setup(_instance);

		// Create window surface.
		_windowHandler = info.windowHandler;
		_surface = _windowHandler->CreateSurface(_instance);

		// Set up hardware.
		_physicalDevice.Setup(info, _instance, _surface);

		// Set up hardware interface.
		_logicalDevice.Setup(info, _surface, _physicalDevice);
	}

	VkCore::~VkCore()
	{
		DeviceWaitIdle();

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

	void VkCore::Debugger::Setup(const Instance& instance)
	{
		#ifdef NDEBUG
		return;
		#endif

		auto createInfo = CreateInfo();
		const auto result = CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &value);
		assert(!result);
	}

	void VkCore::Debugger::Cleanup(const Instance& instance) const
	{
		#ifdef NDEBUG
		return;
		#endif

		DestroyDebugUtilsMessengerEXT(instance, value, nullptr);
	}

	VkCore::Debugger::operator VkDebugUtilsMessengerEXT() const
	{
		return value;
	}

	void VkCore::Debugger::CheckValidationSupport(const Info& info)
	{
		#ifdef NDEBUG
		return;
		#endif

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		ArrayPtr<VkLayerProperties> availableLayers(layerCount, GMEM_TEMP);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.GetData());

		// Check if validation layers ara available.
		for (const auto& layer : info.validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
				if (strcmp(layer, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}

			assert(layerFound);
		}
	}

	VkDebugUtilsMessengerCreateInfoEXT VkCore::Debugger::CreateInfo()
	{
		VkDebugUtilsMessengerCreateInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = DebugCallback;
		return info;
	}

	void VkCore::Debugger::EnableValidationLayers(const Info& info,
		VkDebugUtilsMessengerCreateInfoEXT& debugInfo,
		VkInstanceCreateInfo& instanceInfo)
	{
		auto& validationLayers = info.validationLayers;
		instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.GetCount());
		instanceInfo.ppEnabledLayerNames = validationLayers.GetData();
		instanceInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugInfo);
	}

	VkBool32 VkCore::Debugger::DebugCallback(
		const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		const VkDebugUtilsMessageTypeFlagsEXT messageType, 
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	VkResult VkCore::Debugger::CreateDebugUtilsMessengerEXT(
		const VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (func)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void VkCore::Debugger::DestroyDebugUtilsMessengerEXT(
		const VkInstance instance, const 
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
	{
		const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (func)
			func(instance, debugMessenger, pAllocator);
	}

	void VkCore::Instance::Setup(const Info& info, const Debugger& debugger)
	{
		auto appInfo = CreateApplicationInfo(info);
		const auto extensions = GetExtensions(info);

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.GetLength());
		createInfo.ppEnabledExtensionNames = extensions.GetData();

		auto validationCreateInfo = debugger.CreateInfo();
		debugger.EnableValidationLayers(info, validationCreateInfo, createInfo);

		const auto result = vkCreateInstance(&createInfo, nullptr, &value);
		assert(!result);
	}

	void VkCore::Instance::Cleanup() const
	{
		vkDestroyInstance(value, nullptr);
	}

	VkCore::Instance::operator VkInstance() const
	{
		return value;
	}

	VkApplicationInfo VkCore::Instance::CreateApplicationInfo(const Info& info)
	{
		const auto& windowInfo = info.windowHandler->GetInfo();
		const auto& name = windowInfo.name.GetData();
		const auto version = VK_MAKE_VERSION(1, 0, 0);

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = name;
		appInfo.applicationVersion = version;
		appInfo.pEngineName = name;
		appInfo.engineVersion = version;
		appInfo.apiVersion = VK_API_VERSION_1_0;

		return appInfo;
	}

	ArrayPtr<const char*> VkCore::Instance::GetExtensions(const Info& info)
	{
		const auto requiredExtensions = info.windowHandler->GetRequiredExtensions();
		auto& additionalExtensions = info.additionalExtensions;

		uint32_t debugExtensions = 0;
		#ifdef _DEBUG
		debugExtensions = 1;
		#endif

		// Merge all extensions into one array.
		const size_t size = requiredExtensions.GetLength() + additionalExtensions.GetCount() + debugExtensions;
		ArrayPtr<const char*> extensions(size, GMEM_TEMP);
		extensions.CopyData(requiredExtensions, 0, requiredExtensions.GetLength());
		extensions.CopyData(info.additionalExtensions, requiredExtensions.GetLength(), size - debugExtensions);

		#ifdef _DEBUG
		extensions[extensions.GetLength() - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		#endif

		return extensions;
	}

	VkCore::PhysicalDevice::QueueFamilies::operator bool() const
	{
		for (const auto& family : values)
			if (family == UINT32_MAX)
				return false;
		return true;
	}

	void VkCore::PhysicalDevice::Setup(const Info& info, const Instance& instance, const VkSurfaceKHR& surface)
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

	void VkCore::LogicalDevice::Setup(const Info& info, VkSurfaceKHR surface, const PhysicalDevice& physicalDevice)
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
