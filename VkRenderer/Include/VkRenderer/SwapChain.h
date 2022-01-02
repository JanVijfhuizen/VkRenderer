#pragma once
#include "Queues.h"
#include "ArrayPtr.h"

namespace vi
{
	class VkRenderer;

	class SwapChain final
	{
	public:
		friend VkRenderer;

		struct SupportDetails final
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats{};
			std::vector<VkPresentModeKHR> presentModes{};

			[[nodiscard]] explicit operator bool() const;
			[[nodiscard]] uint32_t GetRecommendedImageCount() const;
		};

		void SetRenderPass(VkRenderPass renderPass);
		void BeginFrame(bool callWaitForImage = true);
		void EndFrame(bool& shouldRecreateAssets);

		void WaitForImage();

		[[nodiscard]] VkFormat GetFormat() const;
		[[nodiscard]] VkExtent2D GetExtent() const;
		[[nodiscard]] uint32_t GetImageCount() const;
		[[nodiscard]] uint32_t GetCurrentImageIndex() const;
		[[nodiscard]] VkRenderPass GetRenderPass() const;

		[[nodiscard]] static SupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

	private:
		#define _MAX_FRAMES_IN_FLIGHT 2

		struct Info final
		{
			VkSurfaceKHR surface;
			VkPhysicalDevice physicalDevice;
			VkDevice device;
			Queues queues;
			VkCommandPool commandPool;

			class WindowHandler* windowHandler;
			VkRenderer* renderer;
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
				VkImageView imageViews[2];
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

		Info _info;

		VkExtent2D _extent;
		VkFormat _format;
		VkSwapchainKHR _swapChain;

		ArrayPtr<Frame> _frames;
		ArrayPtr<Image> _images;
		ArrayPtr<VkFence> _imagesInFlight;

		VkRenderPass _renderPass = VK_NULL_HANDLE;
		
		uint32_t _frameIndex = 0;
		uint32_t _imageIndex;

		void Construct(const Info& info);
		void Cleanup();

		void CreateImages();
		void CreateSyncObjects();
		void CreateBuffers();
		void CleanupBuffers();

		[[nodiscard]] VkResult Present();

		[[nodiscard]] static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		[[nodiscard]] static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		[[nodiscard]] VkExtent2D ChooseExtent(const Info& info, const VkSurfaceCapabilitiesKHR& capabilities) const;
	};
}
