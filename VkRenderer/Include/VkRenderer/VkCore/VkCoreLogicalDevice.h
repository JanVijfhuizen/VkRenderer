#pragma once

namespace vi
{
	struct VkCorePhysicalDevice;
	struct VkCoreInfo;
	class VkCore;

	/// <summary>
	/// Contains the queue handles for the various command types.
	/// </summary>
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
	/// Handles the interface created from the selected GPU.
	/// </summary>
	struct VkCoreLogicalDevice final
	{
		friend VkCore;

	public:
		Queues queues;

		void Setup(const VkCoreInfo& info, VkSurfaceKHR surface, const VkCorePhysicalDevice& physicalDevice);
		void Cleanup() const;
		operator VkDevice() const;

	private:
		VkDevice _value;
	};
}