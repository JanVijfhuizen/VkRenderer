#pragma once

namespace vi
{
	struct VkCoreInfo;
	struct VkCoreDebugger;

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