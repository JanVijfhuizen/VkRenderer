#pragma once

namespace vi
{
	struct VkCoreInfo;
	struct VkCoreDebugger;

	/// <summary>
	/// Class used by VkCore to set up and manage parts of the renderer.
	/// </summary>
	struct VkCoreInstance final
	{
		friend class VkCore;

	public:
		void Setup(const VkCoreInfo& info);
		void Cleanup() const;
		operator VkInstance() const;

		[[nodiscard]] static VkApplicationInfo CreateApplicationInfo(const VkCoreInfo& info);
		[[nodiscard]] static ArrayPtr<const char*> GetExtensions(const VkCoreInfo& info);

	private:
		VkInstance value;

		VkCoreInstance();
	};
}