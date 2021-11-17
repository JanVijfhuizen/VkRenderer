#include "pch.h"
#include "VkRenderer.h"
#include "InstanceFactory.h"
#include "WindowHandler.h"
#include "PhysicalDeviceFactory.h"
#include "LogicalDeviceFactory.h"

namespace vi
{
	VkRenderer::VkRenderer(const Settings& settings) : _windowHandler(settings.windowHandler)
	{
		Debugger::Info info;
		info.settings = settings.debugger;
		info.instance = &_instance;
		_debugger = new Debugger(info);

		InstanceFactory::Info instanceInfo;
		instanceInfo.settings = settings.instance;
		instanceInfo.debugger = _debugger;
		instanceInfo.windowHandler = _windowHandler;
		_instance = InstanceFactory::Construct(instanceInfo);

		_debugger->CreateDebugMessenger();

		_surface = _windowHandler->CreateSurface(_instance);

		PhysicalDeviceFactory::Info physicalDeviceInfo;
		physicalDeviceInfo.instance = _instance;
		physicalDeviceInfo.surface = _surface;
		physicalDeviceInfo.deviceExtensions = settings.deviceExtensions;
		_physicalDevice = PhysicalDeviceFactory::Construct(physicalDeviceInfo);

		LogicalDeviceFactory::Info logicalDeviceInfo;
		logicalDeviceInfo.surface = _surface;
		logicalDeviceInfo.physicalDevice = _physicalDevice;
		logicalDeviceInfo.deviceExtensions = settings.deviceExtensions;
		logicalDeviceInfo.validationLayers = settings.debugger.validationLayers;
		logicalDeviceInfo.debugger = _debugger;
		const auto outLogicalDeviceFactory = LogicalDeviceFactory::Construct(logicalDeviceInfo);
		_device = outLogicalDeviceFactory.device;
		_queues = outLogicalDeviceFactory.queues;

		const auto families = PhysicalDeviceFactory::GetQueueFamilies(_surface, _physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = families.graphics;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		const auto result = vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool);
		assert(!result);
	}

	VkRenderer::~VkRenderer()
	{
		DeviceWaitIdle();

		vkDestroyCommandPool(_device, _commandPool, nullptr);
		vkDestroyDevice(_device, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		delete _debugger;
		vkDestroyInstance(_instance, nullptr);
	}

	void VkRenderer::Submit(VkCommandBuffer* buffers, 
		const uint32_t buffersCount, const VkSemaphore waitSemaphore,
		const VkSemaphore signalSemaphore, const VkFence fence) const
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.waitSemaphoreCount = waitSemaphore ? 1 : 0;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.commandBufferCount = buffersCount;
		submitInfo.pCommandBuffers = buffers;
		submitInfo.signalSemaphoreCount = signalSemaphore ? 1 : 0;
		submitInfo.pSignalSemaphores = &signalSemaphore;

		vkResetFences(_device, 1, &fence);
		const auto result = vkQueueSubmit(_queues.graphics, 1, &submitInfo, fence);
		assert(!result);
	}

	void VkRenderer::DeviceWaitIdle() const
	{
		const auto result = vkDeviceWaitIdle(_device);
		assert(!result);
	}

	VkImage VkRenderer::CreateImage(
		const glm::ivec2 resolution, 
		const VkFormat format, 
		const VkImageTiling tiling,
		const VkImageUsageFlags usage) const
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(resolution.x);
		imageInfo.extent.height = static_cast<uint32_t>(resolution.y);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkImage image;
		const auto result = vkCreateImage(_device, &imageInfo, nullptr, &image);
		assert(!result);
		return image;
	}

	void VkRenderer::DestroyImage(const VkImage image) const
	{
		vkDestroyImage(_device, image, nullptr);
	}

	VkImageView VkRenderer::CreateImageView(const VkImage image, 
		const VkFormat format, const VkImageAspectFlags aspectFlags) const
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		const auto result = vkCreateImageView(_device, &createInfo, nullptr, &imageView);
		assert(!result);
		return imageView;
	}

	void VkRenderer::DestroyImageView(const VkImageView imageView) const
	{
		vkDestroyImageView(_device, imageView, nullptr);
	}

	VkFramebuffer VkRenderer::CreateFrameBuffer(const VkImageView* imageViews, const uint32_t imageViewCount,
		const VkRenderPass renderPass, const VkExtent2D extent) const
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = imageViewCount;
		framebufferInfo.pAttachments = imageViews;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		VkFramebuffer frameBuffer;
		const auto result = vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &frameBuffer);
		assert(!result);
		return frameBuffer;
	}

	void VkRenderer::DestroyFrameBuffer(const VkFramebuffer frameBuffer) const
	{
		vkDestroyFramebuffer(_device, frameBuffer, nullptr);
	}

	void VkRenderer::BeginRenderPass(const VkFramebuffer frameBuffer, 
		const VkRenderPass renderPass, const glm::ivec2 offset,
		const glm::ivec2 extent, VkClearValue* clearColors, const uint32_t clearColorsCount) const
	{
		const VkExtent2D extentVk
		{
			static_cast<uint32_t>(extent.x),
			static_cast<uint32_t>(extent.y)
		};

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = frameBuffer;
		renderPassInfo.renderArea.offset = { offset.x, offset.y };
		renderPassInfo.renderArea.extent = extentVk;

		renderPassInfo.clearValueCount = clearColorsCount;
		renderPassInfo.pClearValues = clearColors;

		vkCmdBeginRenderPass(_currentCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VkRenderer::EndRenderPass() const
	{
		vkCmdEndRenderPass(_currentCommandBuffer);
	}

	VkCommandBuffer VkRenderer::CreateCommandBuffer() const
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = _commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		const auto result = vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);
		assert(!result);

		return commandBuffer;
	}

	void VkRenderer::BeginCommandBufferRecording(const VkCommandBuffer commandBuffer)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		_currentCommandBuffer = commandBuffer;
	}

	void VkRenderer::EndCommandBufferRecording() const
	{
		const auto result = vkEndCommandBuffer(_currentCommandBuffer);
		assert(!result);
	}

	void VkRenderer::DestroyCommandBuffer(const VkCommandBuffer commandBuffer) const
	{
		vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
	}

	VkSemaphore VkRenderer::CreateSemaphore() const
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkSemaphore semaphore;
		const auto result = vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &semaphore);
		assert(!result);
		return semaphore;
	}

	void VkRenderer::DestroySemaphore(const VkSemaphore semaphore) const
	{
		vkDestroySemaphore(_device, semaphore, nullptr);
	}

	VkFence VkRenderer::CreateFence() const
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkFence fence;
		const auto result = vkCreateFence(_device, &fenceInfo, nullptr, &fence);
		assert(!result);
		return fence;
	}

	void VkRenderer::WaitForFence(const VkFence fence) const
	{
		vkWaitForFences(_device, 1, &fence, VK_TRUE, UINT64_MAX);
	}

	void VkRenderer::DestroyFence(const VkFence fence) const
	{
		vkDestroyFence(_device, fence, nullptr);
	}
}
