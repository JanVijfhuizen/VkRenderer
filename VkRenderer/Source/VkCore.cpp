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

		_instance.Setup(info, _debugger);
		_debugger.Setup(_instance);
		_physicalDevice.Setup(_instance);
	}

	VkCore::~VkCore()
	{
		_debugger.Cleanup(_instance);
		_instance.Cleanup();
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

	void VkCore::Instance::Setup(const Info& info, Debugger& debugger)
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

	void VkCore::PhysicalDevice::Setup(const Instance& instance)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		assert(deviceCount);

		ArrayPtr<VkPhysicalDevice> devices{deviceCount, GMEM_TEMP};
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.GetData());
		/*
		std::multimap<uint32_t, VkPhysicalDevice> candidates;

		for (const auto& device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			deviceFeatures.samplerAnisotropy = VK_TRUE;

			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			const auto families = GetQueueFamilies(info.surface, device);
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

			if (!IsDeviceSuitable(info, deviceInfo))
				continue;

			const uint32_t score = RateDevice(deviceInfo);
			candidates.insert({ score, device });
		}

		assert(!candidates.empty());
		return candidates.rbegin()->second;
		*/
	}

	VkCore::PhysicalDevice::operator VkPhysicalDevice_T*() const
	{
		return value;
	}
}
