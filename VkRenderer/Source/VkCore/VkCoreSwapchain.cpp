#include "pch.h"
#include "VkCore/VkCoreSwapchain.h"

namespace vi
{
	VkCoreSwapchain::VkCoreSwapchain() = default;

	VkCoreSwapchain::SupportDetails::operator bool() const
	{
		return !formats.IsNull() && !presentModes.IsNull();
	}

	uint32_t VkCoreSwapchain::SupportDetails::GetRecommendedImageCount() const
	{
		uint32_t imageCount = capabilities.minImageCount + 1;

		const auto& maxImageCount = capabilities.maxImageCount;
		if (maxImageCount > 0 && imageCount > maxImageCount)
			imageCount = maxImageCount;

		if (imageCount > SWAPCHAIN_MAX_FRAMES)
			imageCount = SWAPCHAIN_MAX_FRAMES;
		return imageCount;
	}

	VkCoreSwapchain::SupportDetails VkCoreSwapchain::QuerySwapChainSupport(
		const VkSurfaceKHR surface,
		const VkPhysicalDevice device)
	{
		SupportDetails details{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			auto& formats = details.formats;
			formats = ArrayPtr<VkSurfaceFormatKHR>(formatCount, GMEM_TEMP);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.GetData());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			auto& presentModes = details.presentModes;
			presentModes = ArrayPtr<VkPresentModeKHR>(presentModeCount, GMEM_TEMP);

			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
				&presentModeCount, details.presentModes.GetData());
		}

		return details;
	}
}