#include "pch.h"
#include "VkCore/VkCoreDebugger.h"
#include "VkCore/VkCoreInfo.h"
#include "VkCore/VkCoreInstance.h"

namespace vi
{
	VkCoreDebugger::VkCoreDebugger() = default;

	void VkCoreDebugger::Setup(const VkCoreInstance& instance)
	{
#ifdef NDEBUG
		return;
#endif

		auto createInfo = CreateInfo();
		const auto result = CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &_value);
		assert(!result);
	}

	void VkCoreDebugger::Cleanup(const VkCoreInstance& instance) const
	{
#ifdef NDEBUG
		return;
#endif

		DestroyDebugUtilsMessengerEXT(instance, _value, nullptr);
	}

	VkCoreDebugger::operator VkDebugUtilsMessengerEXT_T*() const
	{
		return _value;
	}

	void VkCoreDebugger::CheckValidationSupport(const VkCoreInfo& info)
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

	VkDebugUtilsMessengerCreateInfoEXT VkCoreDebugger::CreateInfo()
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

	void VkCoreDebugger::EnableValidationLayers(
		const VkCoreInfo& info, 
		VkDebugUtilsMessengerCreateInfoEXT& debugInfo,
		VkInstanceCreateInfo& instanceInfo)
	{
		auto& validationLayers = info.validationLayers;
		instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.GetCount());
		instanceInfo.ppEnabledLayerNames = validationLayers.GetData();
		instanceInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugInfo);
	}

	VkBool32 VkCoreDebugger::DebugCallback(
		const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		const VkDebugUtilsMessageTypeFlagsEXT messageType, 
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	VkResult VkCoreDebugger::CreateDebugUtilsMessengerEXT(
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

	void VkCoreDebugger::DestroyDebugUtilsMessengerEXT(
		const VkInstance instance, 
		const VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
	{
		const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (func)
			func(instance, debugMessenger, pAllocator);
	}
}
