#pragma once

#include "VkCoreInfo.h"
#include "VkCoreDebugger.h"
#include "VkCoreInstance.h"

namespace vi
{
	class WindowHandler;

	/// <summary>
	/// Sets up vulkan in a way where it can be used for generic game development purposes.
	/// </summary>
	class VkCore final
	{
	public:
		explicit VkCore(VkCoreInfo& info);
		~VkCore();

		void DeviceWaitIdle() const;

	private:
		WindowHandler* _windowHandler;
		VkSurfaceKHR _surface;

		VkCoreDebugger _debugger;
		VkCoreInstance _instance;

		/// <summary>
		/// Contains all the physical device related methods and variables.
		/// </summary>
		struct PhysicalDevice final
		{
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

			VkPhysicalDevice value;

			void Setup(const VkCoreInfo& info, const VkCoreInstance& instance, const VkSurfaceKHR& surface);
			operator VkPhysicalDevice() const;

			[[nodiscard]] static QueueFamilies GetQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice physicalDevice);
			[[nodiscard]] static bool IsDeviceSuitable(VkSurfaceKHR surface, const DeviceInfo& deviceInfo);
			[[nodiscard]] static uint32_t RateDevice(const DeviceInfo& deviceInfo);
			[[nodiscard]] static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const ArrayPtr<const char*>& extensions);
		} _physicalDevice;

		struct LogicalDevice final 
		{
			VkDevice value;

			union
			{
				struct
				{
					VkQueue graphics;
					VkQueue present;
				};
				VkQueue queues[2];
			};

			void Setup(const VkCoreInfo& info, VkSurfaceKHR surface, const PhysicalDevice& physicalDevice);
			void Cleanup() const;
			operator VkDevice() const;
		} _logicalDevice;

		struct CommandPool final
		{
			VkCommandPool value;

			void Setup(VkSurfaceKHR surface, const PhysicalDevice& physicalDevice, const LogicalDevice& logicalDevice);
			void Cleanup(const LogicalDevice& logicalDevice) const;
			operator VkCommandPool() const;
		} _commandPool;

		struct SwapChain final
		{
			struct SupportDetails final
			{
				VkSurfaceCapabilitiesKHR capabilities;
				ArrayPtr<VkSurfaceFormatKHR> formats;
				ArrayPtr<VkPresentModeKHR> presentModes;

				[[nodiscard]] explicit operator bool() const;
				[[nodiscard]] uint32_t GetRecommendedImageCount() const;
			};

			[[nodiscard]] static SupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);
		};
	};
}
