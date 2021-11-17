#include "pch.h"
#include "VkRenderer.h"
#include "InstanceFactory.h"
#include "WindowHandler.h"
#include "PhysicalDeviceFactory.h"

namespace vi
{
	VkRenderer::VkRenderer(const Settings& settings) : _windowHandler(settings.windowHandler)
	{
		#ifdef _DEBUG
		Debugger::Info info;
		info.settings = settings.debugger;
		info.instance = &_instance;
		_debugger = new Debugger(info);
		#endif

		InstanceFactory::Info instanceInfo;
		instanceInfo.settings = settings.instance;
		instanceInfo.debugger = _debugger;
		_instance = InstanceFactory::Construct(instanceInfo);

		#ifdef _DEBUG
		_debugger->CreateDebugMessenger();
		#endif

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
