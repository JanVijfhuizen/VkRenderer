#include "pch.h"
#include "VkRenderer.h"
#include "InstanceFactory.h"
#include "WindowHandler.h"
#include "PhysicalDeviceFactory.h"
#include "LogicalDeviceFactory.h"

namespace vi
{
	VkRenderer::VkRenderer(const Settings& settings) : _windowHandler(settings.windowHandler)
	{
		Debugger::Info info;
		info.settings = settings.debugger;
		info.instance = &_instance;
		_debugger = new Debugger(info);

		InstanceFactory::Info instanceInfo;
		instanceInfo.settings = settings.instance;
		instanceInfo.debugger = _debugger;
		instanceInfo.windowHandler = _windowHandler;
		_instance = InstanceFactory::Construct(instanceInfo);

		_debugger->CreateDebugMessenger();

		_surface = _windowHandler->CreateSurface(_instance);

		PhysicalDeviceFactory::Info physicalDeviceInfo;
		physicalDeviceInfo.instance = _instance;
		physicalDeviceInfo.surface = _surface;
		physicalDeviceInfo.deviceExtensions = settings.deviceExtensions;
		_physicalDevice = PhysicalDeviceFactory::Construct(physicalDeviceInfo);

		LogicalDeviceFactory::Info logicalDeviceInfo;
		logicalDeviceInfo.surface = _surface;
		logicalDeviceInfo.physicalDevice = _physicalDevice;
		logicalDeviceInfo.deviceExtensions = settings.deviceExtensions;
		logicalDeviceInfo.validationLayers = settings.debugger.validationLayers;
		logicalDeviceInfo.debugger = _debugger;
		const auto outLogicalDeviceFactory = LogicalDeviceFactory::Construct(logicalDeviceInfo);
		_device = outLogicalDeviceFactory.device;
		_queues = outLogicalDeviceFactory.queues;

		const auto families = PhysicalDeviceFactory::GetQueueFamilies(_surface, _physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = families.graphics;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		const auto result = vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool);
		assert(!result);
	}

	VkRenderer::~VkRenderer()
	{
		DeviceWaitIdle();

		vkDestroyCommandPool(_device, _commandPool, nullptr);
		vkDestroyDevice(_device, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		delete _debugger;
		vkDestroyInstance(_instance, nullptr);
	}

	void VkRenderer::DeviceWaitIdle() const
	{
		const auto result = vkDeviceWaitIdle(_device);
		assert(!result);
	}
}
