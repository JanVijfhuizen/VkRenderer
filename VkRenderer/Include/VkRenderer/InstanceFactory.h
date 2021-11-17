#pragma once

namespace vi
{
	class InstanceFactory final
	{
	public:
		struct Settings
		{
			std::vector<const char*> additionalExtensions{};
		};

		struct Info final
		{
			Settings settings;
			class WindowHandler* windowHandler;
			class Debugger* debugger;
		};

		[[nodiscard]] static VkInstance Construct(const Info& info);

	private:
		[[nodiscard]] static VkApplicationInfo CreateApplicationInfo(const Info& info);
		[[nodiscard]] static std::vector<const char*> GetExtensions(const Info& info);
	};
}