#pragma once

namespace vi
{
	struct VkCoreInfo;
	struct VkCoreInstance;

	struct VkCorePhysicalDevice final
	{
		friend class VkCore;

	public:
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

			[[nodiscard]] explicit operator bool() const;
		};

		struct DeviceInfo final
		{
			VkPhysicalDevice device;
			VkPhysicalDeviceProperties properties;
			VkPhysicalDeviceFeatures features;
		};

		void Setup(const VkCoreInfo& info, const VkCoreInstance& instance, const VkSurfaceKHR& surface);
		operator VkPhysicalDevice() const;

		[[nodiscard]] static QueueFamilies GetQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice physicalDevice);
		[[nodiscard]] static bool IsDeviceSuitable(VkSurfaceKHR surface, const DeviceInfo& deviceInfo);
		[[nodiscard]] static uint32_t RateDevice(const DeviceInfo& deviceInfo);
		[[nodiscard]] static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const ArrayPtr<const char*>& extensions);

	private:
		VkPhysicalDevice _value;

		VkCorePhysicalDevice();
	};
}
