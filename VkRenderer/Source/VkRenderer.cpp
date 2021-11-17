#include "pch.h"
#include "VkRenderer.h"
#include "InstanceFactory.h"

namespace vi
{
	VkRenderer::VkRenderer(const Settings& settings)
	{
		#ifdef _DEBUG
		Debugger::Info info;
		info.settings = settings.debugger;
		info.instance = &_instance;
		_debugger = new Debugger(info);
		#endif

		InstanceFactory::Info instanceInfo{};
		instanceInfo.settings = settings.instance;
		_instance = InstanceFactory::Construct(instanceInfo);

		#ifdef _DEBUG
		_debugger->CreateDebugMessenger();
		#endif
	}

	VkRenderer::~VkRenderer()
	{
		delete _debugger;
		InstanceFactory::Cleanup(_instance);
	}
}
