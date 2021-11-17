#pragma once

namespace vi
{
	class PhysicalDeviceFactory final
	{
	public:
		struct Info final
		{
			VkInstance instance;
			VkSurfaceKHR surface;
			std::vector<const char*> deviceExtensions;
		};

		struct DeviceInfo final
		{
			VkPhysicalDevice device;
			VkPhysicalDeviceProperties properties;
			VkPhysicalDeviceFeatures features;
		};

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

		[[nodiscard]] static VkPhysicalDevice Construct(const Info& info);
		[[nodiscard]] static QueueFamilies GetQueueFamilies(const VkSurfaceKHR& surface, VkPhysicalDevice physicalDevice);

	private:
		[[nodiscard]] static bool IsDeviceSuitable(const Info& info, const DeviceInfo& deviceInfo);
		[[nodiscard]] static uint32_t RateDevice(const DeviceInfo& deviceInfo);
		[[nodiscard]] static bool CheckDeviceExtensionSupport(VkPhysicalDevice device,
			const std::vector<const char*>& extensions);
	};
}