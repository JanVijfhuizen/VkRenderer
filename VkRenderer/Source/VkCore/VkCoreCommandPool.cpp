#include "pch.h"
#include "VkCore/VkCoreCommandPool.h"
#include "VkCore/VkCorePhysicalDevice.h"
#include "VkCore/VkCoreLogicalDevice.h"

namespace vi
{
	void VkCoreCommandPool::Setup(
		const VkSurfaceKHR surface, 
		const VkCorePhysicalDevice& physicalDevice,
		const VkCoreLogicalDevice& logicalDevice)
	{
		const auto families = VkCorePhysicalDevice::GetQueueFamilies(surface, physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = families.graphics;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		const auto result = vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &_value);
		assert(!result);
	}

	void VkCoreCommandPool::Cleanup(const VkCoreLogicalDevice& logicalDevice) const
	{
		vkDestroyCommandPool(logicalDevice, _value, nullptr);
	}

	VkCoreCommandPool::operator VkCommandPool_T*() const
	{
		return _value;
	}

	VkCoreCommandPool::VkCoreCommandPool() = default;
}
