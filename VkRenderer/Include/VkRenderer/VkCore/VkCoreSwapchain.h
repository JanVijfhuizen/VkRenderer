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

		ArrayPtr<Image> _images;
		ArrayPtr<Frame> _frames;
		ArrayPtr<VkFence> _inFlight;
		VkFormat _format;

		VkExtent2D _extent;
		VkSwapchainKHR _swapChain = VK_NULL_HANDLE;

		VkCoreSwapchain();

		void Construct(VkCore& core);
		void Reconstruct(VkCore& core);
		void Cleanup(VkCore& core) const;

		[[nodiscard]] static SupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

		void ConstructImages(VkCore& core) const;
		void ConstructFrames(VkCore& core) const;

		void FreeImages(VkCore& core) const;
		void FreeFrames(VkCore& core) const;

		[[nodiscard]] static VkSurfaceFormatKHR ChooseSurfaceFormat(const ArrayPtr<VkSurfaceFormatKHR>& availableFormats);
		[[nodiscard]] static VkPresentModeKHR ChoosePresentMode(const ArrayPtr<VkPresentModeKHR>& availablePresentModes);
		[[nodiscard]] static VkExtent2D ChooseExtent(const WindowHandler& windowHandler, const VkSurfaceCapabilitiesKHR& capabilities);
	};
}
