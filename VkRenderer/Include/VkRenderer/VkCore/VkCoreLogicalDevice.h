#pragma once

namespace vi
{
	struct VkCoreInfo;
	struct VkCorePhysicalDevice;

	struct Queues final
	{
		union
		{
			struct
			{
				VkQueue graphics;
				VkQueue present;
			};
			VkQueue values[2];
		};
	};

	/// <summary>
	/// Class used by VkCore to set up and manage parts of the renderer.
	/// </summary>
	struct VkCoreLogicalDevice final
	{
		friend class VkCore;

	public:
		Queues queues;

		void Setup(const VkCoreInfo& info, VkSurfaceKHR surface, const VkCorePhysicalDevice& physicalDevice);
		void Cleanup() const;
		operator VkDevice() const;

	private:
		VkDevice _value;

		VkCoreLogicalDevice();
	};
}