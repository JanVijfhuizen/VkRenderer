#pragma once

namespace vi
{
	struct VkCoreLogicalDevice;
	struct VkCorePhysicalDevice;
	class VkCore;
	class WindowHandler;

	/// <summary>
	/// Handles drawing directly to the screen.
	/// </summary>
	class VkCoreSwapchain final
	{
	public:
		friend VkCore;
		friend VkCorePhysicalDevice;

		explicit VkCoreSwapchain(VkCore& core);

		/// <summary>
		/// Call this at the start of the frame.
		/// </summary>
		/// <param name="callWaitForImage">If false, does not call wait for image. <br>
		/// Useful for when you want to do some rendering before actually drawing to the swap chain image.</param>
		void BeginFrame(bool callWaitForImage = true);
		/// <summary>
		/// Call this at the end of the frame.
		/// </summary>
		void EndFrame(VkSemaphore overrideWaitSemaphore = VK_NULL_HANDLE);

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
		[[nodiscard]] glm::ivec2 GetExtent() const;
		/// <returns>Depth buffer image format.</returns>
		[[nodiscard]] VkFormat GetDepthBufferFormat() const;
		/// <returns>Color image format.</returns>
		[[nodiscard]] VkFormat GetFormat() const;
		/// <returns>Render pass in use by the swapchain.</returns>
		[[nodiscard]] VkRenderPass GetRenderPass() const;
		/// <returns>If the current render assets are outdated by the swapchain.</returns>
		[[nodiscard]] VkSemaphore GetImageAvaiableSemaphore() const;
		[[nodiscard]] bool GetShouldRecreateAssets() const;

	private:
#define _MAX_FRAMES_IN_FLIGHT 3

		/// <summary>
		/// Contains the details to create the swap chain with.
		/// </summary>
		struct SupportDetails final
		{
			VkSurfaceCapabilitiesKHR capabilities;
			ArrayPtr<VkSurfaceFormatKHR> formats;
			ArrayPtr<VkPresentModeKHR> presentModes;

			[[nodiscard]] explicit operator bool() const;
			// Returns the recommended amount of images in the swap chain.
			[[nodiscard]] uint32_t GetRecommendedImageCount() const;
		};

		/// <summary>
		/// Swap chain image, write to this to draw directly to the screen.
		/// </summary>
		struct Image final
		{
			union
			{
				struct
				{
					// Color image view.
					VkImageView imageView;
					// Depth image view.
					VkImageView depthImageView;
				};
				VkImageView imageViews[2]
				{
					VK_NULL_HANDLE,
					VK_NULL_HANDLE
				};
			};

			// Color image.
			VkImage image;
			// Depth image.
			VkImage depthImage;
			VkDeviceMemory depthImageMemory;

			// Render target.
			VkFramebuffer frameBuffer;
			// Reusable command buffer for the drawing operation.
			VkCommandBuffer commandBuffer;
		};

		/// <summary>
		/// Handles synchronization across multiple swap chain images that may or may not be in flight.
		/// </summary>
		struct Frame final
		{
			// Triggers once the image is available.
			VkSemaphore imageAvailableSemaphore;
			// Triggers once the image is finished rendering.
			VkSemaphore renderFinishedSemaphore;
			// Triggers once the image is no longer in flight.
			VkFence inFlightFence;
		};

		VkCore& _core;

		ArrayPtr<Image> _images;
		ArrayPtr<Frame> _frames;
		// Fences for the images that are currently in flight. fences can be null.
		ArrayPtr<VkFence> _inFlight;

		// Color image format.
		VkFormat _format;
		// Depth image format.
		VkFormat _depthBufferFormat;

		// Resolution of the swap chain.
		VkExtent2D _extent;
		VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
		// Assigned render pass for the swap chain.
		VkRenderPass _renderPass = VK_NULL_HANDLE;

		// Index for the frames (linear and separated from the images).
		uint32_t _frameIndex = 0;
		// Index for the images (can be nonlinear).
		uint32_t _imageIndex;
		// Is true if the swapchain is no longer the correct shape, like when the window has been resized.
		VkResult _shouldRecreateAssets;

		void Construct();
		void Cleanup() const;
		void IntReconstruct(bool executeCleanup = true);

		// Render the current image to the screen (with a bit of delay).
		[[nodiscard]] VkResult Present();

		[[nodiscard]] static SupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

		void ConstructImages() const;
		void ConstructFrames() const;
		void ConstructBuffers() const;

		void FreeImages() const;
		void FreeFrames() const;
		void FreeBuffers() const;

		// Choose color formatting for the swap chain images, like RGB(A).
		[[nodiscard]] static VkSurfaceFormatKHR ChooseSurfaceFormat(const ArrayPtr<VkSurfaceFormatKHR>& availableFormats);
		// Choose the way the swapchain presents the images.
		[[nodiscard]] static VkPresentModeKHR ChoosePresentMode(const ArrayPtr<VkPresentModeKHR>& availablePresentModes);
		// Choose the resolution for the swap chain images.
		[[nodiscard]] static VkExtent2D ChooseExtent(const WindowHandler& windowHandler, const VkSurfaceCapabilitiesKHR& capabilities);
	};
}
