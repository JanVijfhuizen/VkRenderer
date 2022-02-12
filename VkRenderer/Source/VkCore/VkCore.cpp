#include "pch.h"
#include "VkCore/VkCore.h"
#include "WindowHandler.h"

#include "VkCore/VkCoreInfo.h"

#include "VkCore/VkCoreInstance.h"
#include "VkCore/VkCoreDebugger.h"
#include "VkCore/VkCorePhysicalDevice.h"
#include "VkCore/VkCoreLogicalDevice.h"
#include "VkCore/VkCoreCommandPool.h"
#include "VkCore/VkCoreSwapchain.h"

#include "VkHandlers/VkCommandBufferHandler.h"
#include "VkHandlers/VkDescriptorPoolHandler.h"
#include "VkHandlers/VkFrameBufferHandler.h"
#include "VkHandlers/VkImageHandler.h"
#include "VkHandlers/VkLayoutHandler.h"
#include "VkHandlers/VkMemoryHandler.h"
#include "VkHandlers/VkPipelineHandler.h"
#include "VkHandlers/VkRenderPassHandler.h"
#include "VkHandlers/VkShaderHandler.h"
#include "VkHandlers/VkSyncHandler.h"

namespace vi
{
	VkCore::VkCore(VkCoreInfo& info)
	{
		assert(info.windowHandler);

		_debugger = GMEM.New<VkCoreDebugger>();
		_instance = GMEM.New<VkCoreInstance>();
		_physicalDevice = GMEM.New<VkCorePhysicalDevice>();
		_logicalDevice = GMEM.New<VkCoreLogicalDevice>();
		_commandPool = GMEM.New<VkCoreCommandPool>();
		_swapChain = GMEM.New<VkCoreSwapchain>(*this);

		_commandBufferHandler = GMEM.New<VkCommandBufferHandler>(*this);
		_descriptorPoolHandler = GMEM.New<VkDescriptorPoolHandler>(*this);
		_frameBufferHandler = GMEM.New<VkFrameBufferHandler>(*this);
		_imageHandler = GMEM.New<VkImageHandler>(*this);
		_layoutHandler = GMEM.New<VkLayoutHandler>(*this);
		_memoryHandler = GMEM.New<VkMemoryHandler>(*this);
		_pipelineHandler = GMEM.New<VkPipelineHandler>(*this);
		_renderPassHandler = GMEM.New<VkRenderPassHandler>(*this);
		_shaderHandler = GMEM.New<VkShaderHandler>(*this);
		_syncHandler = GMEM.New<VkSyncHandler>(*this);

		// Add required tags.
		info.deviceExtensions.Add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		info.validationLayers.Add("VK_LAYER_KHRONOS_validation");
		// Check debugging support.
		VkCoreDebugger::CheckValidationSupport(info);

		// Set up vulkan instance.
		_instance->Setup(info);
		_debugger->Setup(*_instance);

		// Create window surface.
		_windowHandler = info.windowHandler;
		_surface = _windowHandler->CreateSurface(*_instance);

		_physicalDevice->Setup(info, *_instance, _surface);
		_logicalDevice->Setup(info, _surface, *_physicalDevice);
		_commandPool->Setup(_surface, *_physicalDevice, *_logicalDevice);
		_swapChain->Construct();
	}

	VkCore::~VkCore()
	{
		DeviceWaitIdle();

		_swapChain->Cleanup();
		_commandPool->Cleanup(*_logicalDevice);
		_logicalDevice->Cleanup();
		vkDestroySurfaceKHR(*_instance, _surface, nullptr);
		_debugger->Cleanup(*_instance);
		_instance->Cleanup();

		GMEM.Delete(_commandBufferHandler);
		GMEM.Delete(_descriptorPoolHandler);
		GMEM.Delete(_frameBufferHandler);
		GMEM.Delete(_imageHandler);
		GMEM.Delete(_layoutHandler);
		GMEM.Delete(_memoryHandler);
		GMEM.Delete(_pipelineHandler);
		GMEM.Delete(_renderPassHandler);
		GMEM.Delete(_shaderHandler);
		GMEM.Delete(_syncHandler);

		GMEM.Delete(_debugger);
		GMEM.Delete(_instance);
		GMEM.Delete(_physicalDevice);
		GMEM.Delete(_logicalDevice);
		GMEM.Delete(_commandPool);
	}

	void VkCore::DeviceWaitIdle() const
	{
		const auto result = vkDeviceWaitIdle(*_logicalDevice);
		assert(!result);
	}

	VkSurfaceKHR VkCore::GetSurface() const
	{
		return _surface;
	}

	VkInstance VkCore::GetInstance() const
	{
		return *_instance;
	}

	VkPhysicalDevice VkCore::GetPhysicalDevice() const
	{
		return *_physicalDevice;
	}

	VkDevice VkCore::GetLogicalDevice() const
	{
		return *_logicalDevice;
	}

	Queues VkCore::GetQueues() const
	{
		return _logicalDevice->queues;
	}

	VkCommandPool VkCore::GetCommandPool() const
	{
		return *_commandPool;
	}

	WindowHandler& VkCore::GetWindowHandler() const
	{
		return *_windowHandler;
	}

	VkCoreSwapchain& VkCore::GetSwapChain() const
	{
		return *_swapChain;
	}

	VkCommandBufferHandler& VkCore::GetCommandBufferHandler() const
	{
		return *_commandBufferHandler;
	}

	VkDescriptorPoolHandler& VkCore::GetDescriptorPoolHandler() const
	{
		return *_descriptorPoolHandler;
	}

	VkFrameBufferHandler& VkCore::GetFrameBufferHandler() const
	{
		return *_frameBufferHandler;
	}

	VkImageHandler& VkCore::GetImageHandler() const
	{
		return *_imageHandler;
	}

	VkLayoutHandler& VkCore::GetLayoutHandler() const
	{
		return *_layoutHandler;
	}

	VkMemoryHandler& VkCore::GetMemoryHandler() const
	{
		return *_memoryHandler;
	}

	VkPipelineHandler& VkCore::GetPipelineHandler() const
	{
		return *_pipelineHandler;
	}

	VkRenderPassHandler& VkCore::GetRenderPassHandler() const
	{
		return *_renderPassHandler;
	}

	VkShaderHandler& VkCore::GetShaderHandler() const
	{
		return *_shaderHandler;
	}

	VkSyncHandler& VkCore::GetSyncHandler() const
	{
		return *_syncHandler;
	}
}
