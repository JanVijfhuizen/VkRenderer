#pragma once

namespace vi
{
	class VkCoreSwapchain final
	{
		friend class VkCore;

	public:
		struct SupportDetails final
		{
			VkSurfaceCapabilitiesKHR capabilities;
			ArrayPtr<VkSurfaceFormatKHR> formats;
			ArrayPtr<VkPresentModeKHR> presentModes;

			[[nodiscard]] explicit operator bool() const;
			[[nodiscard]] uint32_t GetRecommendedImageCount() const;
		};

		[[nodiscard]] static SupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

	private:
		VkCoreSwapchain();
	};
}
