#pragma once

namespace vi
{
	class Debugger final
	{
	public:
		struct Settings final
		{
			std::vector<const char*> validationLayers =
			{
				"VK_LAYER_KHRONOS_validation"
			};
		};

		struct Info final
		{
			Settings settings;
			VkInstance* instance;
		};

		explicit Debugger(Info info);
		~Debugger();

		void EnableValidationLayers(VkInstanceCreateInfo& instanceInfo) const;
		void CreateDebugMessenger();

	private:
		VkDebugUtilsMessengerEXT _debugMessenger;
		Info _info;

		[[nodiscard]] static VkDebugUtilsMessengerCreateInfoEXT CreateInfo();

		void CheckValidationSupport() const;

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
	};
}
