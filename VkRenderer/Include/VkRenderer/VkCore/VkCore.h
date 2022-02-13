#pragma once

namespace vi
{
	struct Queues;
	class VkCoreCommandPool;
	class VkCoreDebugger;
	class VkCoreInstance;
	class VkCoreLogicalDevice;
	class VkCorePhysicalDevice;
	class VkCoreSwapchain;
	class VkCommandBufferHandler;
	class VkDescriptorPoolHandler;
	class VkFrameBufferHandler;
	class VkImageHandler;
	class VkLayoutHandler;
	class VkMemoryHandler;
	class VkPipelineHandler;
	class VkRenderPassHandler;
	class VkShaderHandler;
	class VkSyncHandler;
	class WindowHandler;

	/// <summary>
	/// Sets up vulkan in a way where it can be used for generic game development purposes.
	/// </summary>
	class VkCore
	{
	public:
		explicit VkCore(class VkCoreInfo& info);
		~VkCore();

		/// <summary>
		/// Wait until all graphic related tasks are finished.
		/// </summary>
		void DeviceWaitIdle() const;

		[[nodiscard]] VkSurfaceKHR GetSurface() const;
		[[nodiscard]] VkInstance GetInstance() const;
		[[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const;
		[[nodiscard]] VkDevice GetLogicalDevice() const;
		[[nodiscard]] Queues GetQueues() const;
		[[nodiscard]] VkCommandPool GetCommandPool() const;

		[[nodiscard]] WindowHandler& GetWindowHandler() const;
		[[nodiscard]] VkCoreSwapchain& GetSwapChain() const;

		[[nodiscard]] VkCommandBufferHandler& GetCommandBufferHandler() const;
		[[nodiscard]] VkDescriptorPoolHandler& GetDescriptorPoolHandler() const;
		[[nodiscard]] VkFrameBufferHandler& GetFrameBufferHandler() const;
		[[nodiscard]] VkImageHandler& GetImageHandler() const;
		[[nodiscard]] VkLayoutHandler& GetLayoutHandler() const;
		[[nodiscard]] VkMemoryHandler& GetMemoryHandler() const;
		[[nodiscard]] VkPipelineHandler& GetPipelineHandler() const;
		[[nodiscard]] VkRenderPassHandler& GetRenderPassHandler() const;
		[[nodiscard]] VkShaderHandler& GetShaderHandler() const;
		[[nodiscard]] VkSyncHandler& GetSyncHandler() const;

	private:
		WindowHandler* _windowHandler;
		VkSurfaceKHR _surface;

		VkCoreDebugger* _debugger;
		VkCoreInstance* _instance;
		VkCorePhysicalDevice* _physicalDevice;
		VkCoreLogicalDevice* _logicalDevice;
		VkCoreCommandPool* _commandPool;
		VkCoreSwapchain* _swapChain;

		VkCommandBufferHandler* _commandBufferHandler;
		VkDescriptorPoolHandler* _descriptorPoolHandler;
		VkFrameBufferHandler* _frameBufferHandler;
		VkImageHandler* _imageHandler;
		VkLayoutHandler* _layoutHandler;
		VkMemoryHandler* _memoryHandler;
		VkPipelineHandler* _pipelineHandler;
		VkRenderPassHandler* _renderPassHandler;
		VkShaderHandler* _shaderHandler;
		VkSyncHandler* _syncHandler;
	};
}
