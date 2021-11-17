#include "pch.h"
#include "VkRenderer.h"
#include "InstanceFactory.h"
#include "WindowHandler.h"
#include "PhysicalDeviceFactory.h"

namespace vi
{
	VkRenderer::VkRenderer(const Settings& settings) : _windowHandler(settings.windowHandler), _settings(settings)
	{
		Debugger::Info info;
		info.settings = _settings.debugger;
		info.instance = &_instance;
		_debugger = new Debugger(info);

		InstanceFactory::Info instanceInfo;
		instanceInfo.settings = _settings.instance;
		instanceInfo.debugger = _debugger;
		instanceInfo.windowHandler = _windowHandler;
		_instance = InstanceFactory::Construct(instanceInfo);

		_debugger->CreateDebugMessenger();

		_surface = _windowHandler->CreateSurface(_instance);

		PhysicalDeviceFactory::Info physicalDeviceInfo;
		_physicalDevice = PhysicalDeviceFactory::Construct(physicalDeviceInfo);
	}

	VkRenderer::~VkRenderer()
	{
		DeviceWaitIdle();

		delete _debugger;
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyInstance(_instance, nullptr);
	}

	void VkRenderer::DeviceWaitIdle() const
	{
		//const auto result = vkDeviceWaitIdle(_device);
		//assert(!result);
	}
}
