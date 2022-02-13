#pragma once

namespace vi
{
	struct VkCoreInfo;
	struct VkCoreInstance;
	class VkCore;

	/// <summary>
	/// [Editor Only] Handles the editor-only debugging side of Vulkan.
	/// </summary>
	struct VkCoreDebugger final
	{
		friend VkCore;

	public:
		void Setup(const VkCoreInstance& instance);
		void Cleanup(const VkCoreInstance& instance) const;
		operator VkDebugUtilsMessengerEXT() const;

		// Throws an error if validation layers are not supported.
		static void CheckValidationSupport(const VkCoreInfo& info);
		[[nodiscard]] static VkDebugUtilsMessengerCreateInfoEXT CreateInfo();
		// Enables debugging layers.
		static void EnableValidationLayers(const VkCoreInfo& info, 
			VkDebugUtilsMessengerCreateInfoEXT& debugInfo, VkInstanceCreateInfo& instanceInfo);

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

	private:
		VkDebugUtilsMessengerEXT _value;
	};
}
