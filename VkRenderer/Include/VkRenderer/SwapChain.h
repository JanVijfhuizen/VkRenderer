#pragma once
#include "Queues.h"

namespace vi
{
	class SwapChain final
	{
	public:
		struct Info final
		{
			VkSurfaceKHR surface;
			VkPhysicalDevice physicalDevice;
			VkDevice device;
			Queues queues;
			VkCommandPool commandPool;

			class WindowHandler* windowHandler;
			class VkRenderer* renderer;
		};

		struct SupportDetails final
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats{};
			std::vector<VkPresentModeKHR> presentModes{};

			[[nodiscard]] explicit operator bool() const;
			[[nodiscard]] uint32_t GetRecommendedImageCount() const;
		};

		explicit SwapChain(const Info& info);
		~SwapChain();

		void SetRenderPass(VkRenderPass renderPass);
		void BeginFrame();
		void EndFrame(bool& shouldRecreateAssets);

		[[nodiscard]] VkFormat GetFormat() const;
		[[nodiscard]] VkExtent2D GetExtent() const;
		[[nodiscard]] uint32_t GetImageCount() const;
		[[nodiscard]] uint32_t GetCurrentImageIndex() const;

		[[nodiscard]] static SupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

	private:
		#define _MAX_FRAMES_IN_FLIGHT 2

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

		std::vector<Frame> _frames{};
		std::vector<Image> _images{};
		std::vector<VkFence> _imagesInFlight{};

		VkRenderPass _renderPass;
		uint32_t _frameIndex = 0;
		uint32_t _imageIndex;

		void CreateImages();
		void CreateSyncObjects();
		void CreateBuffers();
		void CleanupBuffers();
		void WaitForImage();

		[[nodiscard]] VkResult Present();

		[[nodiscard]] static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		[[nodiscard]] static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		[[nodiscard]] VkExtent2D ChooseExtent(const Info& info, const VkSurfaceCapabilitiesKHR& capabilities) const;
	};
}
