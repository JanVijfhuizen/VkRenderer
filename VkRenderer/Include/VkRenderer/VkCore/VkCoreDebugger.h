#pragma once

namespace vi
{
	struct VkCoreInfo;
	struct VkCoreInstance;

	/// <summary>
	/// Class used by VkCore to set up and manage parts of the renderer.
	/// </summary>
	struct VkCoreDebugger final
	{
		friend class VkCore;

	public:
		void Setup(const VkCoreInstance& instance);
		void Cleanup(const VkCoreInstance& instance) const;
		operator VkDebugUtilsMessengerEXT() const;

		static void CheckValidationSupport(const VkCoreInfo& info);
		[[nodiscard]] static VkDebugUtilsMessengerCreateInfoEXT CreateInfo();
		static void EnableValidationLayers(const VkCoreInfo& info, VkDebugUtilsMessengerCreateInfoEXT& debugInfo, VkInstanceCreateInfo& instanceInfo);

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

		VkCoreDebugger();
	};
}
