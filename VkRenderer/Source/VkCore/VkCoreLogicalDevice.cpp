#include "pch.h"
#include "VkCore/VkCoreLogicalDevice.h"
#include "VkCore/VkCoreInfo.h"
#include "VkCore/VkCorePhysicalDevice.h"

namespace vi
{
	void VkCoreLogicalDevice::Setup(
		const VkCoreInfo& info, 
		const VkSurfaceKHR surface,
		const VkCorePhysicalDevice& physicalDevice)
	{
		const auto queueFamilies = VkCorePhysicalDevice::GetQueueFamilies(surface, physicalDevice);

		const uint32_t queueFamiliesCount = sizeof queueFamilies.values / sizeof(uint32_t);
		Vector<VkDeviceQueueCreateInfo> queueCreateInfos{ queueFamiliesCount, GMEM_TEMP };
		HashMap<uint32_t> familyIndexes{ queueFamiliesCount, GMEM_TEMP };
		const float queuePriority = 1;

		// Create a queue for each individual queue family.
		// So if a single family handles both graphics and presentation, only create it once.
		// Hence the hashmap.
		for (auto& family : queueFamilies.values)
		{
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
		deviceFeatures.sampleRateShading = VK_TRUE;

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

		const auto result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &_value);
		assert(!result);

		uint32_t i = 0;
		for (const auto& family : queueFamilies.values)
		{
			vkGetDeviceQueue(_value, family, 0, &queues.values[i]);
			i++;
		}
	}

	void VkCoreLogicalDevice::Cleanup() const
	{
		vkDestroyDevice(_value, nullptr);
	}

	VkCoreLogicalDevice::operator VkDevice_T*() const
	{
		return _value;
	}
}
