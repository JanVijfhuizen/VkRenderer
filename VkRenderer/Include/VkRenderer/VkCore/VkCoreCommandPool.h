#pragma once

namespace vi
{
	struct VkCorePhysicalDevice;
	struct VkCoreLogicalDevice;
	class VkCore;

	/// <summary>
	/// Handles commands like image renders, transitions and transfers.
	/// </summary>
	struct VkCoreCommandPool final
	{
		friend VkCore;

	public:
		void Setup(VkSurfaceKHR surface, const VkCorePhysicalDevice& physicalDevice, const VkCoreLogicalDevice& logicalDevice);
		void Cleanup(const VkCoreLogicalDevice& logicalDevice) const;
		operator VkCommandPool() const;

	private:
		VkCommandPool _value;
	};
}