#pragma once

namespace vi
{
	struct VkCoreInfo;
	struct VkCoreDebugger;
	class VkCore;

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