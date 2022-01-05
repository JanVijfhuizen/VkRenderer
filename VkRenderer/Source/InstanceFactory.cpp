#include "pch.h"
#include "InstanceFactory.h"
#include "WindowHandler.h"
#include "Debugger.h"

namespace vi
{
	VkInstance InstanceFactory::Construct(const Info& info)
	{
		auto appInfo = CreateApplicationInfo(info);
		const auto extensions = GetExtensions(info);

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.GetLength());
		createInfo.ppEnabledExtensionNames = extensions.GetData();

		auto validationCreateInfo = info.debugger->CreateInfo();
		info.debugger->EnableValidationLayers(validationCreateInfo, createInfo);

		VkInstance instance;
		const auto result = vkCreateInstance(&createInfo, nullptr, &instance);
		assert(!result);
		return instance;
	}

	VkApplicationInfo InstanceFactory::CreateApplicationInfo(const Info& info)
	{
		const auto& windowInfo = info.windowHandler->GetInfo();
		const auto& name = windowInfo.name.GetData();
		const auto version = VK_MAKE_VERSION(1, 0, 0);

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = name;
		appInfo.applicationVersion = version;
		appInfo.pEngineName = name;
		appInfo.engineVersion = version;
		appInfo.apiVersion = VK_API_VERSION_1_0;

		return appInfo;
	}

	ArrayPtr<const char*> InstanceFactory::GetExtensions(const Info& info)
	{
		const auto requiredExtensions = info.windowHandler->GetRequiredExtensions();

		uint32_t debugExtensions = 0;
		#ifdef _DEBUG
		debugExtensions = 1;
		#endif

		ArrayPtr<const char*> extensions(requiredExtensions.GetLength() + info.settings.additionalExtensions.size() + debugExtensions, GMEM_TEMP);
		memcpy(extensions.GetData(), requiredExtensions.GetData(), requiredExtensions.GetLength() * sizeof(const char*));

		#ifdef _DEBUG
		extensions[extensions.GetLength() - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		#endif

		return extensions;
	}
}
