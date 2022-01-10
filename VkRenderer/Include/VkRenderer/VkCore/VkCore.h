#pragma once

#include "VkCoreInfo.h"
#include "VkCoreDebugger.h"
#include "VkCoreInstance.h"
#include "VkCorePhysicalDevice.h"
#include "VkCoreLogicalDevice.h"
#include "VkCoreSwapchain.h"
#include "VkCoreCommandPool.h"

#include "../VkHandlers/VkCommandBufferHandler.h"
#include "../VkHandlers/VkDescriptorPoolHandler.h"
#include "../VkHandlers/VkFrameBufferHandler.h"
#include "../VkHandlers/VkImageHandler.h"
#include "../VkHandlers/VkLayoutHandler.h"
#include "../VkHandlers/VkMemoryHandler.h"
#include "../VkHandlers/VkPipelineHandler.h"
#include "../VkHandlers/VkRenderPassHandler.h"
#include "../VkHandlers/VkShaderHandler.h"
#include "../VkHandlers/VkSyncHandler.h"


namespace vi
{
	class WindowHandler;

	/// <summary>
	/// Sets up vulkan in a way where it can be used for generic game development purposes.
	/// </summary>
	class VkCore
	{
	public:
		explicit VkCore(VkCoreInfo& info);
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
		[[nodiscard]] VkCoreSwapchain& GetSwapChain();

		[[nodiscard]] VkCommandBufferHandler& GetCommandBufferHandler();
		[[nodiscard]] VkDescriptorPoolHandler& GetDescriptorPoolHandler();
		[[nodiscard]] VkFrameBufferHandler& GetFrameBufferHandler();
		[[nodiscard]] VkImageHandler& GetImageHandler();
		[[nodiscard]] VkLayoutHandler& GetLayoutHandler();
		[[nodiscard]] VkMemoryHandler& GetMemoryHandler();
		[[nodiscard]] VkPipelineHandler& GetPipelineHandler();
		[[nodiscard]] VkRenderPassHandler& GetRenderPassHandler();
		[[nodiscard]] VkShaderHandler& GetShaderHandler();
		[[nodiscard]] VkSyncHandler& GetSyncHandler();

	private:
		WindowHandler* _windowHandler;
		VkSurfaceKHR _surface;

		VkCoreDebugger _debugger;
		VkCoreInstance _instance;
		VkCorePhysicalDevice _physicalDevice;
		VkCoreLogicalDevice _logicalDevice;
		VkCoreCommandPool _commandPool;
		VkCoreSwapchain _swapChain{*this};

		VkCommandBufferHandler _commandBufferHandler{ *this };
		VkDescriptorPoolHandler _descriptorPoolHandler{ *this };
		VkFrameBufferHandler _frameBufferHandler{ *this };
		VkImageHandler _imageHandler{ *this };
		VkLayoutHandler _layoutHandler{ *this };
		VkMemoryHandler _memoryHandler{ *this };
		VkPipelineHandler _pipelineHandler{ *this };
		VkRenderPassHandler _renderPassHandler{ *this };
		VkShaderHandler _shaderHandler{ *this };
		VkSyncHandler _syncHandler{ *this };
	};
}
