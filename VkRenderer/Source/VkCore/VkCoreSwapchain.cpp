#include "pch.h"
#include "WindowHandler.h"
#include "VkCore/VkCore.h"

namespace vi
{
	VkCoreSwapchain::VkCoreSwapchain() = default;

	VkSurfaceFormatKHR VkCoreSwapchain::ChooseSurfaceFormat(const ArrayPtr<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		return availableFormats[0];
	}

	VkPresentModeKHR VkCoreSwapchain::ChoosePresentMode(const ArrayPtr<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return availablePresentMode;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VkCoreSwapchain::ChooseExtent(const WindowHandler& windowHandler, const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
			return capabilities.currentExtent;

		const auto& resolution = windowHandler.GetInfo().resolution;

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(resolution.x),
			static_cast<uint32_t>(resolution.y)
		};

		const auto& minExtent = capabilities.minImageExtent;
		const auto& maxExtent = capabilities.maxImageExtent;

		actualExtent.width = Ut::Clamp(actualExtent.width, minExtent.width, maxExtent.width);
		actualExtent.height = Ut::Clamp(actualExtent.height, minExtent.height, maxExtent.height);

		return actualExtent;
	}

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

	void VkCoreSwapchain::Construct(VkCore& core)
	{
		const SupportDetails support = QuerySwapChainSupport(core.GetSurface(), core.GetPhysicalDevice());
		const uint32_t imageCount = support.GetRecommendedImageCount();

		// These array's don't ever resize, so reusing these arrays means that there will be no memory fragmentation.
		_images = ArrayPtr<Image>(imageCount, GMEM);
		_frames = ArrayPtr<Frame>(_MAX_FRAMES_IN_FLIGHT, GMEM);
		_inFlight = ArrayPtr<VkFence>(imageCount, GMEM, VK_NULL_HANDLE);

		const VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(support.formats);
		_format = surfaceFormat.format;

		Reconstruct(core);
	}

	void VkCoreSwapchain::Reconstruct(VkCore& core)
	{
		const auto surface = core.GetSurface();
		const auto physicalDevice = core.GetPhysicalDevice();
		const auto logicalDevice = core.GetLogicalDevice();

		const SupportDetails support = QuerySwapChainSupport(surface, physicalDevice);
		const auto families = VkCorePhysicalDevice::GetQueueFamilies(surface, physicalDevice);

		const VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(support.formats);
		const VkPresentModeKHR presentMode = ChoosePresentMode(support.presentModes);

		_extent = ChooseExtent(core.GetWindowHandler(), support.capabilities);

		uint32_t queueFamilyIndices[] =
		{
			families.graphics,
			families.present
		};

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = _images.GetLength();
		createInfo.imageFormat = _format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = _extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (families.graphics != families.present)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		createInfo.preTransform = support.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = _swapChain;

		VkSwapchainKHR newSwapchain;
		const auto result = vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &newSwapchain);
		assert(!result);

		Cleanup(core);

		_swapChain = newSwapchain;

		ConstructImages(core);
		ConstructFrames(core);
	}

	void VkCoreSwapchain::Cleanup(VkCore& core) const
	{
		auto& syncHandler = core.GetSyncHandler();

		for (auto& fence : _inFlight)
			if (fence)
				syncHandler.WaitForFence(fence);

		if(_swapChain)
			vkDestroySwapchainKHR(core.GetLogicalDevice(), _swapChain, nullptr);

		FreeImages(core);
		FreeFrames(core);
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

	void VkCoreSwapchain::ConstructImages(VkCore& core) const
	{
		uint32_t length = _images.GetLength();
		const auto& imageHandler = core.GetImageHandler();

		const auto vkImages = ArrayPtr<VkImage>(length, GMEM_TEMP);
		vkGetSwapchainImagesKHR(core.GetLogicalDevice(), _swapChain, &length, vkImages.GetData());

		for (uint32_t i = 0; i < length; ++i)
		{
			auto& image = _images[i];

			image.image = vkImages[i];
			image.imageView = imageHandler.CreateView(image.image, _format);
		}
	}

	void VkCoreSwapchain::ConstructFrames(VkCore& core) const
	{
		auto& syncHandler = core.GetSyncHandler();

		for (auto& frame : _frames)
		{
			frame.imageAvailableSemaphore = syncHandler.CreateSemaphore();
			frame.renderFinishedSemaphore = syncHandler.CreateSemaphore();
			frame.inFlightFence = syncHandler.CreateFence();
		}
	}

	void VkCoreSwapchain::FreeImages(VkCore& core) const
	{
		const uint32_t length = _images.GetLength();
		const auto& imageHandler = core.GetImageHandler();

		for (uint32_t i = 0; i < length; ++i)
		{
			auto& image = _images[i];
			auto& imageView = image.imageView;

			if (imageView)
				imageHandler.DestroyView(imageView);
		}
	}

	void VkCoreSwapchain::FreeFrames(VkCore& core) const
	{
		auto& syncHandler = core.GetSyncHandler();

		for (const auto& frame : _frames)
		{
			syncHandler.DestroySemaphore(frame.imageAvailableSemaphore);
			syncHandler.DestroySemaphore(frame.renderFinishedSemaphore);
			syncHandler.DestroyFence(frame.inFlightFence);
		}
	}
}
