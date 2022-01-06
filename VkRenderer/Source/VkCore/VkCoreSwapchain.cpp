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

	VkFormat VkCoreSwapchain::GetFormat() const
	{
		return _format;
	}

	VkRenderPass VkCoreSwapchain::GetRenderPass() const
	{
		return _renderPass;
	}

	VkExtent2D VkCoreSwapchain::GetExtent() const
	{
		return _extent;
	}

	VkFormat VkCoreSwapchain::GetDepthBufferFormat() const
	{
		return _depthBufferFormat;
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

		const ArrayPtr<VkFormat> depthFormats{3, GMEM_TEMP};
		depthFormats[0] = VK_FORMAT_D32_SFLOAT;
		depthFormats[1] = VK_FORMAT_D32_SFLOAT_S8_UINT;
		depthFormats[2] = VK_FORMAT_D24_UNORM_S8_UINT;

		_depthBufferFormat = core.GetImageHandler().FindSupportedFormat(depthFormats,
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);

		Reconstruct(core, false);
	}

	void VkCoreSwapchain::Reconstruct(VkCore& core, const bool executeCleanup)
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

		if(executeCleanup)
			Cleanup(core);

		_swapChain = newSwapchain;
		_renderPass = core.GetRenderPassHandler().Create();

		ConstructImages(core);
		ConstructFrames(core);
	}

	void VkCoreSwapchain::Cleanup(VkCore& core) const
	{
		auto& syncHandler = core.GetSyncHandler();

		for (auto& fence : _inFlight)
			if (fence)
				syncHandler.WaitForFence(fence);

		if (_renderPass)
			core.GetRenderPassHandler().Destroy(_renderPass);
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

	void VkCoreSwapchain::ConstructBuffers(VkCore& core) const
	{
		auto& commandBufferHandler = core.GetCommandBufferHandler();
		auto& frameBufferHandler = core.GetFrameBufferHandler();
		auto& imageHandler = core.GetImageHandler();
		auto& memoryHandler = core.GetMemoryHandler();
		auto& syncHandler = core.GetSyncHandler();

		const auto commandPool = core.GetCommandPool();
		const auto logicalDevice = core.GetLogicalDevice();

		const uint32_t length = _images.GetLength();
		const auto commandBuffers = ArrayPtr<VkCommandBuffer>(length, GMEM_TEMP);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = length;

		const auto result = vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.GetData());
		assert(!result);

		for (uint32_t i = 0; i < length; ++i)
		{
			auto& image = _images[i];

			image.depthImage = imageHandler.Create({ _extent.width, _extent.height }, _depthBufferFormat,
				VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
			image.depthImageMemory = memoryHandler.Allocate(image.depthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			memoryHandler.Bind(image.depthImage, image.depthImageMemory);
			image.depthImageView = imageHandler.CreateView(image.depthImage, _depthBufferFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

			auto cmdBuffer = commandBufferHandler.Create();
			const auto fence = syncHandler.CreateFence();

			commandBufferHandler.BeginRecording(cmdBuffer);
			imageHandler.TransitionLayout(image.depthImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
			commandBufferHandler.EndRecording();
			commandBufferHandler.Submit(&cmdBuffer, 1, nullptr, nullptr, fence);
			syncHandler.WaitForFence(fence);

			commandBufferHandler.Destroy(cmdBuffer);
			syncHandler.DestroyFence(fence);

			image.frameBuffer = frameBufferHandler.Create(image.imageViews, 2, _renderPass, _extent);
			image.commandBuffer = commandBuffers[i];
		}
	}

	void VkCoreSwapchain::FreeBuffers(VkCore& core) const
	{
		auto& commandBufferHandler = core.GetCommandBufferHandler();
		auto& frameBufferHandler = core.GetFrameBufferHandler();
		auto& imageHandler = core.GetImageHandler();
		auto& memoryHandler = core.GetMemoryHandler();

		for (auto& image : _images)
		{
			imageHandler.DestroyView(image.depthImageView);
			imageHandler.Destroy(image.depthImage);
			memoryHandler.Free(image.depthImageMemory);

			frameBufferHandler.Destroy(image.frameBuffer);
			commandBufferHandler.Destroy(image.commandBuffer);
		}
	}

	void VkCoreSwapchain::FreeImages(VkCore& core) const
	{
		const uint32_t length = _images.GetLength();
		const auto& imageHandler = core.GetImageHandler();

		for (uint32_t i = 0; i < length; ++i)
		{
			auto& image = _images[i];
			imageHandler.DestroyView(image.imageView);
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
