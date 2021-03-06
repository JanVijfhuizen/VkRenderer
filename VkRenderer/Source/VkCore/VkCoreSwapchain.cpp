#include "pch.h"
#include "VkCore/VkCoreSwapchain.h"
#include "WindowHandler.h"
#include "VkCore/VkCore.h"
#include "VkCore/VkCorePhysicalDevice.h"
#include "VkHandlers/VkCommandBufferHandler.h"
#include "VkHandlers/VkFrameBufferHandler.h"
#include "VkHandlers/VkImageHandler.h"
#include "VkHandlers/VkMemoryHandler.h"
#include "VkHandlers/VkRenderPassHandler.h"
#include "VkHandlers/VkSyncHandler.h"
#include "VkCore/VkCoreLogicalDevice.h"

namespace vi
{
	VkCoreSwapchain::VkCoreSwapchain(VkCore& core) : _core(core)
	{
		
	}

	VkSurfaceFormatKHR VkCoreSwapchain::ChooseSurfaceFormat(const ArrayPtr<VkSurfaceFormatKHR>& availableFormats)
	{
		// Preferably go for SRGB, if it's not present just go with the first one found.
		// We can basically assume that SRGB is supported on most hardware.
		for (const auto& availableFormat : availableFormats)
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		return availableFormats[0];
	}

	VkPresentModeKHR VkCoreSwapchain::ChoosePresentMode(const ArrayPtr<VkPresentModeKHR>& availablePresentModes)
	{
		// Preferably go for Mailbox, otherwise go for Fifo.
		// Fifo is traditional VSync, where mailbox is all that and better, but unlike Fifo is not required to be supported by the hardware.
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

	VkSemaphore VkCoreSwapchain::GetImageAvaiableSemaphore() const
	{
		return _frames[_frameIndex].imageAvailableSemaphore;
	}

	bool VkCoreSwapchain::GetShouldRecreateAssets() const
	{
		return _shouldRecreateAssets;
	}

	void VkCoreSwapchain::BeginFrame(const bool callWaitForImage)
	{
		if (callWaitForImage)
			WaitForImage();

		auto& commandBufferHandler = _core.GetCommandBufferHandler();
		auto& image = _images[_imageIndex];

		// Begin recording the command.
		commandBufferHandler.BeginRecording(image.commandBuffer);

		// Clear the image.
		VkClearValue clearColors[2];
		clearColors[0].color = { 0, 0, 0, 1 };
		clearColors[1].depthStencil = { 1, 0 };

		// Begin the renderpass and clear the previous drawing on the attached image.
		_core.GetRenderPassHandler().Begin(image.frameBuffer, _renderPass, {},
			{ _extent.width, _extent.height }, clearColors, 2);
	}

	VkResult VkCoreSwapchain::Present()
	{
		auto& frame = _frames[_frameIndex];

		// Present the objects drawn during this render pass.
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &frame.renderFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &_swapChain;
		presentInfo.pImageIndices = &_imageIndex;

		const auto result = vkQueuePresentKHR(_core.GetQueues().present, &presentInfo);
		_frameIndex = (_frameIndex + 1) % _frames.GetLength();

		return result;
	}

	void VkCoreSwapchain::EndFrame(const VkSemaphore overrideWaitSemaphore)
	{
		auto& frame = _frames[_frameIndex];
		auto& image = _images[_imageIndex];

		auto& commandBufferHandler = _core.GetCommandBufferHandler();
		auto& renderPassHandler = _core.GetRenderPassHandler();

		renderPassHandler.End();
		commandBufferHandler.EndRecording();

		// Finish the render pass and submit it to the command buffer.
		VkCommandBufferHandler::SubmitInfo info{};
		info.buffers = &image.commandBuffer;
		info.buffersCount = 1;
		info.waitSemaphore = overrideWaitSemaphore ? overrideWaitSemaphore : frame.imageAvailableSemaphore;
		info.signalSemaphore = frame.renderFinishedSemaphore;
		info.fence = frame.inFlightFence;
		commandBufferHandler.Submit(info);

		const auto result = Present();
		_shouldRecreateAssets = result;
	}

	void VkCoreSwapchain::WaitForImage()
	{
		auto& frame = _frames[_frameIndex];
		auto& syncHandler = _core.GetSyncHandler();

		// Wait until the swapchain has a new image available.
		syncHandler.WaitForFence(frame.inFlightFence);
		const auto result = vkAcquireNextImageKHR(_core.GetLogicalDevice(), 
			_swapChain, UINT64_MAX, frame.imageAvailableSemaphore, VK_NULL_HANDLE, &_imageIndex);
		assert(!result);

		auto& imageInFlight = _inFlight[_imageIndex];
		if (imageInFlight != VK_NULL_HANDLE)
			syncHandler.WaitForFence(imageInFlight);
		imageInFlight = frame.inFlightFence;
	}

	void VkCoreSwapchain::Reconstruct()
	{
		IntReconstruct();
	}

	glm::ivec2 VkCoreSwapchain::GetExtent() const
	{
		return { _extent.width, _extent.height };
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
		// Always try to go for one larger than the minimum capability.
		// More swapchain images mean less time waiting for a previous frame to render.
		uint32_t imageCount = capabilities.minImageCount + 1;

		const auto& maxImageCount = capabilities.maxImageCount;
		if (maxImageCount > 0 && imageCount > maxImageCount)
			imageCount = maxImageCount;

		return imageCount;
	}

	void VkCoreSwapchain::Construct()
	{
		const SupportDetails support = QuerySwapChainSupport(_core.GetSurface(), _core.GetPhysicalDevice());
		const uint32_t imageCount = support.GetRecommendedImageCount();

		// These array's don't ever resize, so reusing these arrays means that there will be no memory fragmentation.
		_images = ArrayPtr<Image>(imageCount, GMEM);
		_frames = ArrayPtr<Frame>(_MAX_FRAMES_IN_FLIGHT, GMEM);
		_inFlight = ArrayPtr<VkFence>(imageCount, GMEM, VK_NULL_HANDLE);

		const VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(support.formats);
		_format = surfaceFormat.format;

		// Define the depth buffer formats from which the image handler can choose.
		const ArrayPtr<VkFormat> depthFormats{3, GMEM_TEMP};
		depthFormats[0] = VK_FORMAT_D32_SFLOAT;
		depthFormats[1] = VK_FORMAT_D32_SFLOAT_S8_UINT;
		depthFormats[2] = VK_FORMAT_D24_UNORM_S8_UINT;

		// Choose the depth buffer format for the depth attachments.
		_depthBufferFormat = _core.GetImageHandler().FindSupportedFormat(depthFormats,
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);

		IntReconstruct(false);
	}

	uint32_t VkCoreSwapchain::GetLength() const
	{
		return _images.GetLength();
	}

	uint32_t VkCoreSwapchain::GetImageIndex() const
	{
		return _imageIndex;
	}

	void VkCoreSwapchain::Cleanup() const
	{
		auto& syncHandler = _core.GetSyncHandler();

		// Destroy sync objects.
		for (auto& fence : _inFlight)
		{
			if (fence)
				syncHandler.WaitForFence(fence);
			fence = VK_NULL_HANDLE;
		}

		// Destroy the swap chain and the corresponding render pass.
		_core.GetRenderPassHandler().Destroy(_renderPass);
		vkDestroySwapchainKHR(_core.GetLogicalDevice(), _swapChain, nullptr);

		FreeBuffers();
		FreeImages();
		FreeFrames();
	}

	void VkCoreSwapchain::IntReconstruct(const bool executeCleanup)
	{
		const auto surface = _core.GetSurface();
		const auto physicalDevice = _core.GetPhysicalDevice();
		const auto logicalDevice = _core.GetLogicalDevice();

		const SupportDetails support = QuerySwapChainSupport(surface, physicalDevice);
		const auto families = VkCorePhysicalDevice::GetQueueFamilies(surface, physicalDevice);

		const VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(support.formats);
		const VkPresentModeKHR presentMode = ChoosePresentMode(support.presentModes);

		_extent = ChooseExtent(_core.GetWindowHandler(), support.capabilities);

		uint32_t queueFamilyIndices[] =
		{
			families.graphics,
			families.present
		};

		// Create a new swap chain based on the hardware requirements.
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

		_core.DeviceWaitIdle();

		if (executeCleanup)
			Cleanup();

		_swapChain = newSwapchain;
		// Create a render pass specifically for drawing to the swap chain.
		_renderPass = _core.GetRenderPassHandler().Create();

		ConstructImages();
		ConstructFrames();
		ConstructBuffers();
	}

	VkCoreSwapchain::SupportDetails VkCoreSwapchain::QuerySwapChainSupport(
		const VkSurfaceKHR surface,
		const VkPhysicalDevice device)
	{
		SupportDetails details{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		// Get all supported swap chain formats, if any.
		if (formatCount != 0)
		{
			auto& formats = details.formats;
			formats = ArrayPtr<VkSurfaceFormatKHR>(formatCount, GMEM_TEMP);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.GetData());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		// Get all supported swap chain present modes, if any.
		if (presentModeCount != 0)
		{
			auto& presentModes = details.presentModes;
			presentModes = ArrayPtr<VkPresentModeKHR>(presentModeCount, GMEM_TEMP);

			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
				&presentModeCount, details.presentModes.GetData());
		}

		return details;
	}

	void VkCoreSwapchain::ConstructImages() const
	{
		uint32_t length = _images.GetLength();
		const auto& imageHandler = _core.GetImageHandler();

		const auto vkImages = ArrayPtr<VkImage>(length, GMEM_TEMP);
		vkGetSwapchainImagesKHR(_core.GetLogicalDevice(), _swapChain, &length, vkImages.GetData());

		// Create image views from the color attachments (images are already provided by the swap chain extension).
		for (uint32_t i = 0; i < length; ++i)
		{
			auto& image = _images[i];
			image.image = vkImages[i];

			VkImageHandler::ViewCreateInfo viewCreateInfo{};
			viewCreateInfo.image = image.image;
			viewCreateInfo.format = _format;
			image.imageView = imageHandler.CreateView(viewCreateInfo);
		}
	}

	void VkCoreSwapchain::ConstructFrames() const
	{
		auto& syncHandler = _core.GetSyncHandler();

		// Construct the sync objects.
		for (auto& frame : _frames)
		{
			frame.imageAvailableSemaphore = syncHandler.CreateSemaphore();
			frame.renderFinishedSemaphore = syncHandler.CreateSemaphore();
			frame.inFlightFence = syncHandler.CreateFence();
		}
	}

	void VkCoreSwapchain::ConstructBuffers() const
	{
		auto& commandBufferHandler = _core.GetCommandBufferHandler();
		auto& frameBufferHandler = _core.GetFrameBufferHandler();
		auto& imageHandler = _core.GetImageHandler();
		auto& memoryHandler = _core.GetMemoryHandler();
		auto& syncHandler = _core.GetSyncHandler();

		const auto commandPool = _core.GetCommandPool();
		const auto logicalDevice = _core.GetLogicalDevice();

		const uint32_t length = _images.GetLength();
		const auto commandBuffers = ArrayPtr<VkCommandBuffer>(length, GMEM_TEMP);

		// Create a reusable command buffer.
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

			// Create the depth image attachment.
			VkImageHandler::CreateInfo imageCreateInfo{};
			imageCreateInfo.resolution = { _extent.width, _extent.height };
			imageCreateInfo.format = _depthBufferFormat;
			imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			image.depthImage = imageHandler.Create(imageCreateInfo);
			image.depthImageMemory = memoryHandler.Allocate(image.depthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			memoryHandler.Bind(image.depthImage, image.depthImageMemory);

			// Create the depth image view.
			VkImageHandler::ViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.image = image.depthImage;
			imageViewCreateInfo.format = _depthBufferFormat;
			imageViewCreateInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
			image.depthImageView = imageHandler.CreateView(imageViewCreateInfo);

			// Transition the depth image layout to be that of a depth attachment.
			auto cmdBuffer = commandBufferHandler.Create();
			const auto fence = syncHandler.CreateFence();

			VkImageHandler::TransitionInfo transitionInfo{};
			transitionInfo.image = image.depthImage;
			transitionInfo.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			transitionInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

			commandBufferHandler.BeginRecording(cmdBuffer);
			imageHandler.TransitionLayout(transitionInfo);
			commandBufferHandler.EndRecording();

			// It would be better to do all the swapchain images at once, but the performance gain is neglectable
			// since it only executes once per swapchain recreation, so I'm a bit lazy here.
			VkCommandBufferHandler::SubmitInfo submitInfo{};
			submitInfo.buffers = &cmdBuffer;
			submitInfo.buffersCount = 1;
			submitInfo.fence = fence;
			commandBufferHandler.Submit(submitInfo);
			syncHandler.WaitForFence(fence);

			commandBufferHandler.Destroy(cmdBuffer);
			syncHandler.DestroyFence(fence);

			// Create a render target from the color and depth image views.
			VkFrameBufferHandler::CreateInfo frameBufferCreateInfo{};
			frameBufferCreateInfo.imageViews = image.imageViews;
			frameBufferCreateInfo.imageViewCount = 2;
			frameBufferCreateInfo.renderPass = _renderPass;
			frameBufferCreateInfo.extent = { _extent.width, _extent.height };
			image.frameBuffer = frameBufferHandler.Create(frameBufferCreateInfo);
			image.commandBuffer = commandBuffers[i];
		}
	}

	void VkCoreSwapchain::FreeBuffers() const
	{
		auto& commandBufferHandler = _core.GetCommandBufferHandler();
		auto& frameBufferHandler = _core.GetFrameBufferHandler();
		auto& imageHandler = _core.GetImageHandler();
		auto& memoryHandler = _core.GetMemoryHandler();

		// Destroys the depth image and the frame buffer.
		for (auto& image : _images)
		{
			imageHandler.DestroyView(image.depthImageView);
			imageHandler.Destroy(image.depthImage);
			memoryHandler.Free(image.depthImageMemory);

			frameBufferHandler.Destroy(image.frameBuffer);
			commandBufferHandler.Destroy(image.commandBuffer);
		}
	}

	void VkCoreSwapchain::FreeImages() const
	{
		const uint32_t length = _images.GetLength();
		const auto& imageHandler = _core.GetImageHandler();

		// Destroy the image views.
		for (uint32_t i = 0; i < length; ++i)
		{
			auto& image = _images[i];
			imageHandler.DestroyView(image.imageView);
		}
	}

	void VkCoreSwapchain::FreeFrames() const
	{
		auto& syncHandler = _core.GetSyncHandler();

		// Destroy the sync objects.
		for (const auto& frame : _frames)
		{
			syncHandler.DestroySemaphore(frame.imageAvailableSemaphore);
			syncHandler.DestroySemaphore(frame.renderFinishedSemaphore);
			syncHandler.DestroyFence(frame.inFlightFence);
		}
	}
}
