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
		vi::HashMap<int> map{10, GMEM_TEMP};

		map.Insert(15);
		map.Insert(25);

		map.Erase(25);

		std::cout << map.Contains(25);
	}

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

		for (const auto& [index, instance] : system)
		{
			std::cout << instance << std::endl;
		}
	}

	return EXIT_SUCCESS;
}
