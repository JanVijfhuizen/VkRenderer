#include "pch.h"
#include "VkCore/VkCoreInstance.h"
#include "VkCore/VkCoreDebugger.h"
#include "VkCore/VkCoreInfo.h"
#include "WindowHandler.h"

namespace vi
{
	void VkCoreInstance::Setup(const VkCoreInfo& info)
	{
		auto appInfo = CreateApplicationInfo(info);
		const auto extensions = GetExtensions(info);

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.GetLength());
		createInfo.ppEnabledExtensionNames = extensions.GetData();

		auto validationCreateInfo = VkCoreDebugger::CreateInfo();
		VkCoreDebugger::EnableValidationLayers(info, validationCreateInfo, createInfo);

		const auto result = vkCreateInstance(&createInfo, nullptr, &_value);
		assert(!result);
	}

	void VkCoreInstance::Cleanup() const
	{
		vkDestroyInstance(_value, nullptr);
	}

	VkCoreInstance::operator VkInstance_T*() const
	{
		return _value;
	}

	VkApplicationInfo VkCoreInstance::CreateApplicationInfo(const VkCoreInfo& info)
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

	ArrayPtr<const char*> VkCoreInstance::GetExtensions(const VkCoreInfo& info)
	{
		const auto requiredExtensions = info.windowHandler->GetRequiredExtensions();
		auto& additionalExtensions = info.instanceExtensions;

		uint32_t debugExtensions = 0;
#ifdef _DEBUG
		debugExtensions = 1;
#endif

		// Merge all extensions into one array.
		const size_t size = requiredExtensions.GetLength() + additionalExtensions.GetCount() + debugExtensions;
		ArrayPtr<const char*> extensions(size, GMEM_TEMP);
		extensions.CopyData(requiredExtensions, 0, requiredExtensions.GetLength());
		extensions.CopyData(info.instanceExtensions, requiredExtensions.GetLength(), size - debugExtensions);

#ifdef _DEBUG
		extensions[extensions.GetLength() - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#endif

		return extensions;
	}
}
