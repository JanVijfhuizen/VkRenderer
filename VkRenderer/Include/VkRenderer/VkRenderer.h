#pragma once
#include "Debugger.h"
#include "InstanceFactory.h"
#include "Queues.h"

namespace vi
{
	class SwapChain;

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

		void Draw(uint32_t indexCount) const;
		void Submit(VkCommandBuffer* buffers, uint32_t buffersCount,
			VkSemaphore waitSemaphore = VK_NULL_HANDLE, 
			VkSemaphore signalSemaphore = VK_NULL_HANDLE, 
			VkFence fence = VK_NULL_HANDLE) const;
		void DeviceWaitIdle() const;

		void BindVertexBuffer(VkBuffer buffer) const;
		void BindIndicesBuffer(VkBuffer buffer) const;

		[[nodiscard]] VkShaderModule CreateShaderModule(const std::vector<char>& data) const;
		void DestroyShaderModule(VkShaderModule module) const;

		[[nodiscard]] VkImage CreateImage(glm::ivec2 resolution, 
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, 
			VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, 
			VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) const;
		void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) const;
		void DestroyImage(VkImage image) const;

		[[nodiscard]] VkImageView CreateImageView(VkImage image, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) const;
		void DestroyImageView(VkImageView imageView) const;

		[[nodiscard]] VkSampler CreateSampler(VkFilter magFilter = VK_FILTER_LINEAR, VkFilter minFilter = VK_FILTER_LINEAR) const;
		void DestroySampler(VkSampler sampler) const;

		[[nodiscard]] VkFramebuffer CreateFrameBuffer(const VkImageView* imageViews, 
			uint32_t imageViewCount, VkRenderPass renderPass, VkExtent2D extent) const;
		void DestroyFrameBuffer(VkFramebuffer frameBuffer) const;

		[[nodiscard]] SwapChain& RecreateSwapChain();

		void BeginRenderPass(VkFramebuffer frameBuffer, VkRenderPass renderPass, 
			glm::ivec2 offset, glm::ivec2 extent,
			VkClearValue* clearColors, uint32_t clearColorsCount) const;
		void EndRenderPass() const;

		[[nodiscard]] VkCommandBuffer CreateCommandBuffer() const;
		void BeginCommandBufferRecording(VkCommandBuffer commandBuffer);
		void EndCommandBufferRecording() const;
		void DestroyCommandBuffer(VkCommandBuffer commandBuffer) const;

		[[nodiscard]] VkSemaphore CreateSemaphore() const;
		void DestroySemaphore(VkSemaphore semaphore) const;

		[[nodiscard]] VkFence CreateFence() const;
		void WaitForFence(VkFence fence) const;
		void DestroyFence(VkFence fence) const;

		[[nodiscard]] VkBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags flags) const;
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0) const;
		void CopyBuffer(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height) const;
		void DestroyBuffer(VkBuffer buffer) const;

		[[nodiscard]] VkDeviceMemory AllocateMemory(VkImage image, VkMemoryPropertyFlags flags) const;
		[[nodiscard]] VkDeviceMemory AllocateMemory(VkBuffer buffer, VkMemoryPropertyFlags flags) const;
		[[nodiscard]] VkDeviceMemory AllocateMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags flags) const;

		void BindMemory(VkImage image, VkDeviceMemory memory, VkDeviceSize offset = 0) const;
		void BindMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize offset = 0) const;

		void FreeMemory(VkDeviceMemory memory) const;

		template <typename T>
		void MapMemory(VkDeviceMemory memory, T* input, VkDeviceSize offset);
		template <typename T>
		void UpdatePushConstant(VkPipelineLayout layout, VkFlags flag, const T& input);

		[[nodiscard]] VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
			VkImageTiling tiling, VkFormatFeatureFlags features) const;
		[[nodiscard]] VkFormat GetDepthBufferFormat() const;

	private:
		class WindowHandler* _windowHandler;
		Debugger* _debugger = nullptr;
		SwapChain* _swapChain = nullptr;

		VkInstance _instance;
		VkSurfaceKHR _surface;
		VkPhysicalDevice _physicalDevice;
		VkDevice _device;
		Queues _queues;
		VkCommandPool _commandPool;

		VkCommandBuffer _currentCommandBuffer;

		[[nodiscard]] uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		static void GetLayoutMasks(VkImageLayout layout, VkAccessFlags& outAccessFlags, VkPipelineStageFlags& outPipelineStageFlags);
	};

	template <typename T>
	void VkRenderer::MapMemory(const VkDeviceMemory memory, T* input, const VkDeviceSize offset)
	{
		void* data;
		const uint32_t size = sizeof(T);
		vkMapMemory(_device, memory, offset, size, 0, &data);
		memcpy(data, static_cast<const void*>(input), size);
		vkUnmapMemory(_device, memory);
	}

	template <typename T>
	void VkRenderer::UpdatePushConstant(const VkPipelineLayout layout, const VkFlags flag, const T& input)
	{
		vkCmdPushConstants(_currentCommandBuffer, layout, flag, 0, sizeof(T), &input);
	}
}
