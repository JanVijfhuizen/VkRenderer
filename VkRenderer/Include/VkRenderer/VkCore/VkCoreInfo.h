#pragma once

namespace vi
{
	class WindowHandler;

	/// <summary>
	/// Used for customizing the VkCore class.
	/// </summary>
	struct VkCoreInfo final
	{
		// Handler that sets up the window.
		WindowHandler* windowHandler = nullptr;
		// Extensions for the physical device.
		Vector<const char*> deviceExtensions{ 1, GMEM_TEMP };
		// Validation layers used for debugging.
		Vector<const char*> validationLayers{ 1, GMEM_TEMP };
		// Optional extensions for the vulkan instance.
		Vector<const char*> instanceExtensions{ 0, GMEM_TEMP };
	};
}