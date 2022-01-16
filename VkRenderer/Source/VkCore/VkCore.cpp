#include "pch.h"
#include "VkCore/VkCore.h"
#include "WindowHandler.h"

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

	VkCommandBufferHandler& VkCore::GetCommandBufferHandler()
	{
		return _commandBufferHandler;
	}

	VkDescriptorPoolHandler& VkCore::GetDescriptorPoolHandler()
	{
		return _descriptorPoolHandler;
	}

	VkFrameBufferHandler& VkCore::GetFrameBufferHandler()
	{
		return _frameBufferHandler;
	}

	VkImageHandler& VkCore::GetImageHandler()
	{
		return _imageHandler;
	}

	VkLayoutHandler& VkCore::GetLayoutHandler()
	{
		return _layoutHandler;
	}

	VkMemoryHandler& VkCore::GetMemoryHandler()
	{
		return _memoryHandler;
	}

	VkPipelineHandler& VkCore::GetPipelineHandler()
	{
		return _pipelineHandler;
	}

	VkRenderPassHandler& VkCore::GetRenderPassHandler()
	{
		return _renderPassHandler;
	}

	VkShaderHandler& VkCore::GetShaderHandler()
	{
		return _shaderHandler;
	}

	VkSyncHandler& VkCore::GetSyncHandler()
	{
		return _syncHandler;
	}
}
