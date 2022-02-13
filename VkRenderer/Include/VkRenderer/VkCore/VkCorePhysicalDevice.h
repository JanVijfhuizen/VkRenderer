#pragma once

namespace vi
{
	struct VkCoreInfo;
	struct VkCoreInstance;
	class VkCore;

	/// <summary>
	/// Handles the GPU hardware selection.
	/// </summary>
	struct VkCorePhysicalDevice final
	{
		friend VkCore;

	public:
		/// <summary>
		/// Stores the handles to different vulkan queues.
		/// </summary>
		struct QueueFamilies final
		{
			union
			{
				struct
				{
					uint32_t graphics;
					uint32_t present;
				};

				uint32_t values[2]
				{
					UINT32_MAX,
					UINT32_MAX
				};
			};

			// Returns true if all the queue families are present.
			[[nodiscard]] explicit operator bool() const;
		};

		/// <summary>
		/// Support information regarding the physical device.
		/// </summary>
		struct DeviceInfo final
		{
			VkPhysicalDevice device;
			VkPhysicalDeviceProperties properties;
			VkPhysicalDeviceFeatures features;
		};

		void Setup(const VkCoreInfo& info, const VkCoreInstance& instance, const VkSurfaceKHR& surface);
		operator VkPhysicalDevice() const;

		[[nodiscard]] static QueueFamilies GetQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice physicalDevice);
		// Returns if the device is compatible with the requirements needed to run the game.
		[[nodiscard]] static bool IsDeviceSuitable(VkSurfaceKHR surface, const DeviceInfo& deviceInfo);
		// Give an arbitrary rating to a device. Used to select the most optimal device.
		[[nodiscard]] static uint32_t RateDevice(const DeviceInfo& deviceInfo);
		// Returns if the device can support the selected extensions.
		[[nodiscard]] static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const ArrayPtr<const char*>& extensions);
		// Checks how many samples can be taken from a pixel, useful for anti-aliasing like MSAA.
		[[nodiscard]] static VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice device);

	private:
		VkPhysicalDevice _value;
	};
}
