#pragma once
#include "Debugger.h"
#include "InstanceFactory.h"

namespace vi
{
	class VkRenderer final
	{
	public:
		struct Settings final
		{
			InstanceFactory::Settings instance{};
			Debugger::Settings debugger{};

			class WindowHandler* windowHandler;
		};

		explicit VkRenderer(const Settings& settings = {});
		~VkRenderer();

	private:
		class Debugger* _debugger = nullptr;

		VkInstance _instance;
	};
}
