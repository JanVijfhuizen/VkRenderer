#include "pch.h"
#include "LogicalDeviceFactory.h"
#include "PhysicalDeviceFactory.h"

namespace vi
{
	LogicalDeviceFactory::Out LogicalDeviceFactory::Construct(const Info& info)
	{
		const auto queueFamilies = PhysicalDeviceFactory::GetQueueFamilies(info.surface, info.physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		const float queuePriority = 1.0f;

		std::set<uint32_t> familyIndexes{};

		for (const auto& family : queueFamilies.values)
		{
			if (familyIndexes.find(family) != familyIndexes.end())
				continue;
			familyIndexes.insert(family);

			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = family;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		const auto& deviceExtensions = info.deviceExtensions;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		createInfo.enabledLayerCount = 0;

		#ifdef _DEBUG
		const auto& validationLayers = info.validationLayers;
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		#endif

		Out out{};

		auto& device = out.device;
		const auto result = vkCreateDevice(info.physicalDevice, &createInfo, nullptr, &device);
		assert(!result);

		auto& queues = out.queues;
		uint32_t i = 0;
		for (const auto& family : queueFamilies.values)
		{
			vkGetDeviceQueue(device, family, 0, &queues.values[i]);
			i++;
		}

		return out;
	}
}
