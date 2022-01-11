#pragma once

namespace vi
{
	class WindowHandler;
	struct VkCoreLogicalDevice;
	struct VkCorePhysicalDevice;

	/// <summary>
	/// Used to draw directly to the screen.
	/// </summary>
	class VkCoreSwapchain final
	{
		friend class VkCore;
		friend VkCorePhysicalDevice;

	public:
		/// <summary>
		/// Call this at the start of the frame.
		/// </summary>
		/// <param name="callWaitForImage">If false, does not call wait for image. <br>
		/// Useful for when you want to do some rendering before actually drawing to the swap chain image.</param>
		void BeginFrame(bool callWaitForImage = true);
		/// <summary>
		/// Call this at the end of the frame.
		/// </summary>
		void EndFrame();

		/// <summary>
		/// Waits until a new image target is available. Called by BeginFrame by default.
		/// </summary>
		void WaitForImage();
		/// <summary>
		/// Reconstruct the swap chain. This is needed in the case of a resize.
		/// </summary>
		void Reconstruct();

		/// <returns>Amount of images in the swap chain.</returns>
		[[nodiscard]] uint32_t GetLength() const;
		/// <returns>Current image count.</returns>
		[[nodiscard]] uint32_t GetImageIndex() const;
		/// <returns>Resolution of the images.</returns>
		[[nodiscard]] VkExtent2D GetExtent() const;
		/// <returns>Depth buffer image format.</returns>
		[[nodiscard]] VkFormat GetDepthBufferFormat() const;
		/// <returns>Color image format.</returns>
		[[nodiscard]] VkFormat GetFormat() const;
		/// <returns>Render pass in use by the swapchain.</returns>
		[[nodiscard]] VkRenderPass GetRenderPass() const;
		/// <returns>If the current render assets are outdated by the swapchain.</returns>
		[[nodiscard]] bool GetShouldRecreateAssets() const;

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
		VkResult _shouldRecreateAssets;

		explicit VkCoreSwapchain(VkCore& core);

		void Construct();
		void Cleanup() const;
		void IntReconstruct(bool executeCleanup = true);

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
