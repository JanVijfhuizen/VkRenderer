#pragma once

namespace vi
{
	struct VkCorePhysicalDevice;
	struct VkCoreLogicalDevice;

	struct VkCoreCommandPool final
	{
		friend class VkCore;

	public:
		void Setup(VkSurfaceKHR surface, const VkCorePhysicalDevice& physicalDevice, const VkCoreLogicalDevice& logicalDevice);
		void Cleanup(const VkCoreLogicalDevice& logicalDevice) const;
		operator VkCommandPool() const;

	private:
		VkCommandPool value;

		VkCoreCommandPool();
	};
}