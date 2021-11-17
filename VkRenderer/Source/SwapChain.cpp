#include "pch.h"
#include "SwapChain.h"

namespace vi
{
	SwapChain::SupportDetails::operator bool() const
	{
		return !formats.empty() && !presentModes.empty();
	}

	uint32_t SwapChain::SupportDetails::GetRecommendedImageCount() const
	{
		uint32_t imageCount = capabilities.minImageCount + 1;

		const auto& maxImageCount = capabilities.maxImageCount;
		if (maxImageCount > 0 && imageCount > maxImageCount)
			imageCount = maxImageCount;

		return imageCount;
	}

	SwapChain::SupportDetails SwapChain::QuerySwapChainSupport(const VkSurfaceKHR surface, const VkPhysicalDevice device)
	{
		SupportDetails details{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
				&presentModeCount, details.presentModes.data());
		}

		return details;
	}
}
