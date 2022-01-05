#pragma once

namespace vi
{
	class WindowHandler;

	/// <summary>
	/// Sets up vulkan in a way where it can be used for generic game development purposes.
	/// </summary>
	class VkCore final
	{
	public:
		struct Info final
		{
			// Handler that sets up the window.
			WindowHandler* windowHandler = nullptr;
			// Extensions for the physical device.
			Vector<const char*> deviceExtensions{1, GMEM_TEMP};
			// Validation layers used for debugging.
			Vector<const char*> validationLayers{1, GMEM_TEMP};
			// Optional extensions for the vulkan instance.
			Vector<const char*> additionalExtensions{0, GMEM_TEMP};
		};

		explicit VkCore(Info& info);
		~VkCore();

		void DeviceWaitIdle() const;

	private:
		WindowHandler* _windowHandler;
		VkSurfaceKHR _surface;

		struct Instance;

		/// <summary>
		/// Contains all the debug related methods and variables.
		/// </summary>
		struct Debugger final
		{
			VkDebugUtilsMessengerEXT value;

			void Setup(const Instance& instance);
			void Cleanup(const Instance& instance) const;
			operator VkDebugUtilsMessengerEXT() const;

			static void CheckValidationSupport(const Info& info);
			[[nodiscard]] static VkDebugUtilsMessengerCreateInfoEXT CreateInfo();
			static void EnableValidationLayers(const Info& info, VkDebugUtilsMessengerCreateInfoEXT& debugInfo, VkInstanceCreateInfo& instanceInfo);

			[[nodiscard]] static VkBool32 DebugCallback(
				VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageType,
				const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void* pUserData);
			[[nodiscard]] static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
				const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
				const VkAllocationCallbacks* pAllocator,
				VkDebugUtilsMessengerEXT* pDebugMessenger);
			static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
				VkDebugUtilsMessengerEXT debugMessenger,
				const VkAllocationCallbacks* pAllocator);	
		} _debugger;

		/// <summary>
		/// Contains all the instance related methods and variables.
		/// </summary>
		struct Instance final
		{
			VkInstance value;

			void Setup(const Info& info, const Debugger& debugger);
			void Cleanup() const;
			operator VkInstance() const;

			[[nodiscard]] static VkApplicationInfo CreateApplicationInfo(const Info& info);
			[[nodiscard]] static ArrayPtr<const char*> GetExtensions(const Info& info);		
		} _instance;

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

			void Setup(const Info& info, const Instance& instance, const VkSurfaceKHR& surface);
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

			void Setup(const Info& info, VkSurfaceKHR surface, const PhysicalDevice& physicalDevice);
			void Cleanup() const;
			operator VkDevice() const;
		} _logicalDevice;

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
