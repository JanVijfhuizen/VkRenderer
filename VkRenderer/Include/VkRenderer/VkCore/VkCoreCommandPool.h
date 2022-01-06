#pragma once

namespace vi
{
	struct VkCorePhysicalDevice;
	struct VkCoreLogicalDevice;

	/// <summary>
	/// Class used by VkCore to set up and manage parts of the renderer.
	/// </summary>
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