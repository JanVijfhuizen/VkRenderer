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

	private:
		struct Instance;

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

		struct Instance final
		{
			VkInstance value;

			void Setup(const Info& info, Debugger& debugger);
			void Cleanup() const;

			operator VkInstance() const;

			[[nodiscard]] static VkApplicationInfo CreateApplicationInfo(const Info& info);
			[[nodiscard]] static ArrayPtr<const char*> GetExtensions(const Info& info);		
		} _instance;
	};
}
