#pragma once
#include "Debugger.h"
#include "InstanceFactory.h"
#include "Queues.h"

namespace vi
{
	class VkRenderer final
	{
	public:
		struct Settings final
		{
			InstanceFactory::Settings instance{};
			Debugger::Settings debugger{};

			class WindowHandler* windowHandler;

			std::vector<const char*> deviceExtensions =
			{
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
		};

		explicit VkRenderer(const Settings& settings);
		~VkRenderer();

		void Submit(VkCommandBuffer* buffers, uint32_t buffersCount,
			VkSemaphore waitSemaphore = VK_NULL_HANDLE, 
			VkSemaphore signalSemaphore = VK_NULL_HANDLE, 
			VkFence fence = VK_NULL_HANDLE) const;
		void DeviceWaitIdle() const;

		[[nodiscard]] VkImage CreateImage(glm::ivec2 resolution, 
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, 
			VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, 
			VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) const;
		void DestroyImage(VkImage image) const;

		[[nodiscard]] VkImageView CreateImageView(VkImage image, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) const;
		void DestroyImageView(VkImageView imageView) const;

		[[nodiscard]] VkFramebuffer CreateFrameBuffer(const VkImageView* imageViews, 
			uint32_t imageViewCount, VkRenderPass renderPass, VkExtent2D extent) const;
		void DestroyFrameBuffer(VkFramebuffer frameBuffer) const;

		[[nodiscard]] VkCommandBuffer CreateCommandBuffer() const;
		void BeginCommandBufferRecording(VkCommandBuffer commandBuffer);
		void EndCommandBufferRecording() const;
		void DestroyCommandBuffer(VkCommandBuffer commandBuffer) const;

		[[nodiscard]] VkSemaphore CreateSemaphore() const;
		void DestroySemaphore(VkSemaphore semaphore) const;

		[[nodiscard]] VkFence CreateFence() const;
		void WaitForFence(VkFence fence) const;
		void DestroyFence(VkFence fence) const;

	private:
		class WindowHandler* _windowHandler;
		class Debugger* _debugger = nullptr;

		VkInstance _instance;
		VkSurfaceKHR _surface;
		VkPhysicalDevice _physicalDevice;
		VkDevice _device;
		Queues _queues;
		VkCommandPool _commandPool;

		VkCommandBuffer _currentCommandBuffer;
	};
}
