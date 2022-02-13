#pragma once

namespace vi
{
	struct VkCoreInfo;
	struct VkCoreDebugger;
	class VkCore;

	/// <summary>
	/// Handles the Vulkan instance from which to create the renderer.
	/// </summary>
	struct VkCoreInstance final
	{
		friend VkCore;

	public:
		void Setup(const VkCoreInfo& info);
		void Cleanup() const;
		operator VkInstance() const;

		[[nodiscard]] static VkApplicationInfo CreateApplicationInfo(const VkCoreInfo& info);
		[[nodiscard]] static ArrayPtr<const char*> GetExtensions(const VkCoreInfo& info);

	private:
		VkInstance _value;
	};
}