#include "pch.h"
#include "SwapChain.h"
#include "PhysicalDeviceFactory.h"
#include "WindowHandler.h"
#include "VkRenderer.h"
#include "FreeListAllocator.h"

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

		if (imageCount > SWAPCHAIN_MAX_FRAMES)
			imageCount = SWAPCHAIN_MAX_FRAMES;
		return imageCount;
	}

	void SwapChain::Construct(const Info& info)
	{
		_info = info;

		const SupportDetails support = QuerySwapChainSupport(info.surface, info.physicalDevice);
		const auto families = PhysicalDeviceFactory::GetQueueFamilies(info.surface, info.physicalDevice);

		const VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(support.formats);
		const VkPresentModeKHR presentMode = ChoosePresentMode(support.presentModes);

		_extent = ChooseExtent(info, support.capabilities);
		_format = surfaceFormat.format;

		const uint32_t imageCount = support.GetRecommendedImageCount();

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = info.surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = _format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = _extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		uint32_t queueFamilyIndices[] =
		{
			static_cast<uint32_t>(families.graphics),
			static_cast<uint32_t>(families.present)
		};

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
		createInfo.oldSwapchain = VK_NULL_HANDLE; // Todo: store old swapchain.

		const auto result = vkCreateSwapchainKHR(info.device, &createInfo, nullptr, &_swapChain);
		assert(!result);

		const auto memImage = _info.allocator->MAlloc(sizeof(Image) * imageCount);
		const auto memFrames = _info.allocator->MAlloc(sizeof(Frame) * _MAX_FRAMES_IN_FLIGHT);
		const auto memImagesInFlight = _info.allocator->MAlloc(sizeof(VkFence) * imageCount);

		_images = ArrayPtr<Image>(reinterpret_cast<Image*>(memImage), imageCount);
		_frames = ArrayPtr<Frame>(reinterpret_cast<Frame*>(memFrames), _MAX_FRAMES_IN_FLIGHT);
		_imagesInFlight = ArrayPtr<VkFence>(reinterpret_cast<VkFence*>(memImagesInFlight), imageCount);
		for (auto& fence : _imagesInFlight)
			fence = VK_NULL_HANDLE;

		CreateImages();
		CreateSyncObjects();
	}

	void SwapChain::Cleanup()
	{
		const auto renderer = _info.renderer;

		for (auto& fence : _imagesInFlight)
			if (fence)
				renderer->WaitForFence(fence);

		auto& device = _info.device;

		CleanupBuffers();

		for (const auto& image : _images)
			renderer->DestroyImageView(image.imageView);

		for (const auto& frame : _frames)
		{
			renderer->DestroySemaphore(frame.imageAvailableSemaphore);
			renderer->DestroySemaphore(frame.renderFinishedSemaphore);
			renderer->DestroyFence(frame.inFlightFence);
		}

		_info.allocator->Delete(_images.GetData());
		_info.allocator->Delete(_frames.GetData());
		_info.allocator->Delete(_imagesInFlight.GetData());

		vkDestroySwapchainKHR(device, _swapChain, nullptr);
	}

	void SwapChain::SetRenderPass(const VkRenderPass renderPass)
	{
		if(_renderPass != VK_NULL_HANDLE)
			CleanupBuffers();
		_renderPass = renderPass;
		CreateBuffers();
	}

	void SwapChain::BeginFrame(const bool callWaitForImage)
	{
		if(callWaitForImage)
			WaitForImage();

		auto& renderer = _info.renderer;
		auto& image = _images[_imageIndex];

		renderer->BeginCommandBufferRecording(image.commandBuffer);

		VkClearValue clearColors[2];
		clearColors[0].color = { 0, 0, 0, 1 };
		clearColors[1].depthStencil = { 1, 0 };

		renderer->BeginRenderPass(image.frameBuffer, _renderPass, {}, 
			{ _extent.width, _extent.height }, clearColors, 2);
		}

	void SwapChain::EndFrame(bool& shouldRecreateAssets)
	{
		auto& renderer = _info.renderer;
		auto& frame = _frames[_frameIndex];
		auto& image = _images[_imageIndex];

		renderer->EndRenderPass();
		renderer->EndCommandBufferRecording();
		renderer->Submit(&image.commandBuffer, 1, frame.imageAvailableSemaphore, frame.renderFinishedSemaphore, frame.inFlightFence);

		const auto result = Present();
		shouldRecreateAssets = result;
	}

	VkResult SwapChain::Present()
	{
		auto& frame = _frames[_frameIndex];

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &frame.renderFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &_swapChain;
		presentInfo.pImageIndices = &_imageIndex;

		const auto result = vkQueuePresentKHR(_info.queues.present, &presentInfo);
		_frameIndex = (_frameIndex + 1) % _frames.GetSize();

		return result;
	}

	VkFormat SwapChain::GetFormat() const
	{
		return _format;
	}

	VkExtent2D SwapChain::GetExtent() const
	{
		return _extent;
	}

	uint32_t SwapChain::GetImageCount() const
	{
		return _images.GetSize();
	}

	uint32_t SwapChain::GetCurrentImageIndex() const
	{
		return _imageIndex;
	}

	VkRenderPass SwapChain::GetRenderPass() const
	{
		return _renderPass;
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

	void SwapChain::CreateImages()
	{
		const auto renderer = _info.renderer;
		uint32_t count = _images.GetSize();

		std::vector<VkImage> vkImages{};
		vkImages.resize(count);

		vkGetSwapchainImagesKHR(_info.device, _swapChain, &count, vkImages.data());

		for (uint32_t i = 0; i < count; ++i)
		{
			auto& image = _images[i];
			image.image = vkImages[i];
			image.imageView = renderer->CreateImageView(image.image, _format);
		}
	}

	void SwapChain::CreateSyncObjects()
	{
		const auto renderer = _info.renderer;

		for (auto& frame : _frames)
		{
			frame.imageAvailableSemaphore = renderer->CreateSemaphore();
			frame.renderFinishedSemaphore = renderer->CreateSemaphore();
			frame.inFlightFence = renderer->CreateFence();
		}
	}

	void SwapChain::CreateBuffers()
	{
		auto renderer = _info.renderer;
		const uint32_t count = _images.GetSize();

		std::vector<VkCommandBuffer> commandBuffers{};
		commandBuffers.resize(count);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = _info.commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		const auto result = vkAllocateCommandBuffers(_info.device, &allocInfo, commandBuffers.data());
		assert(!result);

		const auto format = renderer->GetDepthBufferFormat();
		const auto extent = GetExtent();

		for (uint32_t i = 0; i < count; ++i)
		{
			auto& image = _images[i];

			image.depthImage = renderer->CreateImage({ extent.width, extent.height }, format,
				VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			image.depthImageMemory = renderer->AllocateMemory(image.depthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			renderer->BindMemory(image.depthImage, image.depthImageMemory);
			image.depthImageView = renderer->CreateImageView(image.depthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT);

			auto cmdBuffer = renderer->CreateCommandBuffer();
			const auto fence = renderer->CreateFence();

			renderer->BeginCommandBufferRecording(cmdBuffer);
			renderer->TransitionImageLayout(image.depthImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
			renderer->EndCommandBufferRecording();
			renderer->Submit(&cmdBuffer, 1, nullptr, nullptr, fence);
			renderer->WaitForFence(fence);

			renderer->DestroyCommandBuffer(cmdBuffer);
			renderer->DestroyFence(fence);

			image.frameBuffer = renderer->CreateFrameBuffer(image.imageViews, 2, _renderPass, _extent);
			image.commandBuffer = commandBuffers[i];
		}
	}

	void SwapChain::CleanupBuffers()
	{
		const auto renderer = _info.renderer;

		for (auto& image : _images)
		{
			renderer->DestroyImageView(image.depthImageView);
			renderer->DestroyImage(image.depthImage);
			renderer->FreeMemory(image.depthImageMemory);

			renderer->DestroyFrameBuffer(image.frameBuffer);
			renderer->DestroyCommandBuffer(image.commandBuffer);
		}
	}

	void SwapChain::WaitForImage()
	{
		const auto renderer = _info.renderer;
		auto& frame = _frames[_frameIndex];

		renderer->WaitForFence(frame.inFlightFence);
		const auto result = vkAcquireNextImageKHR(_info.device, _swapChain, UINT64_MAX, frame.imageAvailableSemaphore, VK_NULL_HANDLE, &_imageIndex);
		assert(!result);

		auto& imageInFlight = _imagesInFlight[_imageIndex];
		if (imageInFlight != VK_NULL_HANDLE)
			renderer->WaitForFence(imageInFlight);
		imageInFlight = frame.inFlightFence;
	}

	VkSurfaceFormatKHR SwapChain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		return availableFormats.front();
	}

	VkPresentModeKHR SwapChain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return availablePresentMode;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChain::ChooseExtent(const Info& info, const VkSurfaceCapabilitiesKHR& capabilities) const
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
			return capabilities.currentExtent;

		const auto& resolution = info.windowHandler->GetVkInfo().resolution;

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(resolution.x),
			static_cast<uint32_t>(resolution.y)
		};

		const auto& minExtent = capabilities.minImageExtent;
		const auto& maxExtent = capabilities.maxImageExtent;

		const uint32_t actualWidth = std::max(minExtent.width, std::min(actualExtent.width, maxExtent.width));
		const uint32_t actualHeight = std::max(minExtent.height, std::min(actualExtent.height, maxExtent.height));
		actualExtent.width = actualWidth;
		actualExtent.height = actualHeight;

		return actualExtent;
	}
}
