#pragma once

namespace vi
{
	class WindowHandler;
	struct VkCoreLogicalDevice;
	struct VkCorePhysicalDevice;

	class VkCoreSwapchain final
	{
		friend class VkCore;
		friend VkCorePhysicalDevice;

	public:
		void BeginFrame(bool callWaitForImage = true);
		void EndFrame(bool& shouldRecreateAssets);

		void WaitForImage();
		void Reconstruct(bool executeCleanup = true);

		[[nodiscard]] VkExtent2D GetExtent() const;
		[[nodiscard]] VkFormat GetDepthBufferFormat() const;
		[[nodiscard]] VkFormat GetFormat() const;
		[[nodiscard]] VkRenderPass GetRenderPass() const;

	private:
#define _MAX_FRAMES_IN_FLIGHT 3

		struct SupportDetails final
		{
			VkSurfaceCapabilitiesKHR capabilities;
			ArrayPtr<VkSurfaceFormatKHR> formats;
			ArrayPtr<VkPresentModeKHR> presentModes;

			[[nodiscard]] explicit operator bool() const;
			[[nodiscard]] uint32_t GetRecommendedImageCount() const;
		};

		struct Image final
		{
			union
			{
				struct
				{
					VkImageView imageView;
					VkImageView depthImageView;
				};
				VkImageView imageViews[2]
				{
					VK_NULL_HANDLE,
					VK_NULL_HANDLE
				};
			};

			VkImage image;
			VkImage depthImage;
			VkDeviceMemory depthImageMemory;

			VkFramebuffer frameBuffer;
			VkCommandBuffer commandBuffer;
		};

		struct Frame final
		{
			VkSemaphore imageAvailableSemaphore;
			VkSemaphore renderFinishedSemaphore;
			VkFence inFlightFence;
		};

		VkCore& _core;

		ArrayPtr<Image> _images;
		ArrayPtr<Frame> _frames;
		ArrayPtr<VkFence> _inFlight;
		VkFormat _format;
		VkFormat _depthBufferFormat;

		VkExtent2D _extent;
		VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
		VkRenderPass _renderPass = VK_NULL_HANDLE;

		uint32_t _frameIndex = 0;
		uint32_t _imageIndex;

		VkCoreSwapchain(VkCore& core);

		void Construct();
		void Cleanup() const;

		[[nodiscard]] VkResult Present();

		[[nodiscard]] static SupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

		void ConstructImages() const;
		void ConstructFrames() const;
		void ConstructBuffers() const;

		void FreeImages() const;
		void FreeFrames() const;
		void FreeBuffers() const;

		[[nodiscard]] static VkSurfaceFormatKHR ChooseSurfaceFormat(const ArrayPtr<VkSurfaceFormatKHR>& availableFormats);
		[[nodiscard]] static VkPresentModeKHR ChoosePresentMode(const ArrayPtr<VkPresentModeKHR>& availablePresentModes);
		[[nodiscard]] static VkExtent2D ChooseExtent(const WindowHandler& windowHandler, const VkSurfaceCapabilitiesKHR& capabilities);
	};
}
