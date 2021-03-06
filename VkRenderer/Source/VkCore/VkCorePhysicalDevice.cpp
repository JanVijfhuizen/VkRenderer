#include "pch.h"
#include "VkCore/VkCorePhysicalDevice.h"
#include "VkCore/VkCoreInstance.h"
#include "VkCore/VkCoreInfo.h"
#include "VkCore/VkCoreSwapchain.h"

namespace vi
{
	VkCorePhysicalDevice::QueueFamilies::operator bool() const
	{
		for (const auto& family : values)
			if (family == UINT32_MAX)
				return false;
		return true;
	}

	void VkCorePhysicalDevice::Setup(
		const VkCoreInfo& info,
		const VkCoreInstance& instance,
		const VkSurfaceKHR& surface)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		assert(deviceCount);

		ArrayPtr<VkPhysicalDevice> devices{ deviceCount, GMEM_TEMP };
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.GetData());

		BinTree<VkPhysicalDevice> candidates{ deviceCount, GMEM_TEMP };

		// Add all potential candidates in a sorted way.
		for (const auto& device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			deviceFeatures.samplerAnisotropy = VK_TRUE;

			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			// If families are not all present, continue.
			const auto families = GetQueueFamilies(surface, device);
			if (!families)
				continue;

			// If extensions are not supported, continue.
			if (!CheckDeviceExtensionSupport(device, info.deviceExtensions))
				continue;

			const DeviceInfo deviceInfo
			{
				device,
				deviceProperties,
				deviceFeatures
			};

			// If the device does not support the chosen features, continue.
			if (!IsDeviceSuitable(surface, deviceInfo))
				continue;

			// Assign hardware score and add it to the potential candidates.
			const int32_t score = RateDevice(deviceInfo);
			candidates.Push({ score, device });
		}

		// Assign the chosen hardware.
		assert(!candidates.IsEmpty());
		_value = candidates.Peek();
	}

	VkCorePhysicalDevice::operator VkPhysicalDevice_T* () const
	{
		return _value;
	}

	VkCorePhysicalDevice::QueueFamilies VkCorePhysicalDevice::GetQueueFamilies(
		const VkSurfaceKHR surface,
		const VkPhysicalDevice physicalDevice)
	{
		QueueFamilies families{};

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		ArrayPtr<VkQueueFamilyProperties> queueFamilies{ queueFamilyCount, GMEM_TEMP };
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.GetData());

		// Check for hardware capabilities.
		uint32_t i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			// If the graphics family is present.
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				families.graphics = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			// Since this is a renderer made for games, we need to be able to render to the screen.
			if (presentSupport)
				families.present = i;

			// If all families have been found.
			if (families)
				break;
			i++;
		}

		return families;
	}

	bool VkCorePhysicalDevice::IsDeviceSuitable(
		const VkSurfaceKHR surface,
		const DeviceInfo& deviceInfo)
	{
		const auto swapChainSupport = VkCoreSwapchain::QuerySwapChainSupport(surface, deviceInfo.device);
		if (!swapChainSupport)
			return false;
		if (!deviceInfo.features.samplerAnisotropy)
			return false;
		if (!deviceInfo.features.geometryShader)
			return false;
		return true;
	}

	uint32_t VkCorePhysicalDevice::RateDevice(const DeviceInfo& deviceInfo)
	{
		uint32_t score = 0;
		auto& properties = deviceInfo.properties;

		// Arbitrary increase in score, not sure what to look for to be honest.
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000;
		// Increase the score based on the maximum resolution supported.
		score += properties.limits.maxImageDimension2D;

		return score;
	}

	bool VkCorePhysicalDevice::CheckDeviceExtensionSupport(
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

		// Remove all the available extensions and check if the hashmap is now empty (which would suggest that all the extensions are supported).
		for (const auto& extension : availableExtensions)
			hashMap.Remove(extension.extensionName);

		return hashMap.IsEmpty();
	}

	VkSampleCountFlagBits VkCorePhysicalDevice::GetMaxUsableSampleCount(const VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

		const VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts
			& physicalDeviceProperties.limits.framebufferDepthSampleCounts;

		// Iterate over all the sample count enums and return the highest supported one.
		VkSampleCountFlagBits ret = VK_SAMPLE_COUNT_1_BIT;
		for (int32_t i = VK_SAMPLE_COUNT_2_BIT; i != VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM; i++)
		{
			const auto flags = static_cast<VkSampleCountFlagBits>(i);
			if (counts & flags)
			{
				ret = flags;
				continue;
			}
			break;
		}

		return ret;
	}
}
