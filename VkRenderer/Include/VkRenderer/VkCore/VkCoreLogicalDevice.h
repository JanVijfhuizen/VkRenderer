#pragma once

namespace vi
{
	struct VkCoreInfo;
	struct VkCorePhysicalDevice;

	struct VkCoreLogicalDevice final
	{
		friend class VkCore;

	public:
		union
		{
			struct
			{
				VkQueue graphics;
				VkQueue present;
			};
			VkQueue queues[2];
		};

		void Setup(const VkCoreInfo& info, VkSurfaceKHR surface, const VkCorePhysicalDevice& physicalDevice);
		void Cleanup() const;
		operator VkDevice() const;

	private:
		VkDevice _value;

		VkCoreLogicalDevice();
	};
}