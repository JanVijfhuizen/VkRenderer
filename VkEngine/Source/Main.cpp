#include "pch.h"
#include "VkRenderer/VkCore/VkCore.h"
#include "VkRenderer/WindowHandlerGLFW.h"
#include "Cecsar.h"
#include "SparseSet.h"

class TransformSystem final : public System<float>
{
public:
	explicit TransformSystem(Cecsar& cecsar);
};

TransformSystem::TransformSystem(Cecsar& cecsar) : System(cecsar)
{
	
}

int main()
{
	{
		vi::WindowHandlerGLFW windowHandler;

		vi::VkCoreInfo info{};
		info.windowHandler = &windowHandler;

		vi::VkCore core{info};

		Cecsar cecsar{100};
		TransformSystem system{cecsar};
		system.Insert(5, 10);
		system.Insert(15, 30);
		system.Insert(2, 4);
		system.RemoveAt(15);

		for (const auto& [instance, index] : system)
		{
			std::cout << instance << std::endl;
		}
	}

	return EXIT_SUCCESS;
}
