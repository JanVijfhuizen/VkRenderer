#include "pch.h"
#include "VkRenderer/VkCore/VkCore.h"
#include "VkRenderer/WindowHandlerGLFW.h"
#include "Cecsar.h"
#include "SparseSet.h"
#include "HashSet.h"

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

		for (const auto& [index, instance] : system)
		{
			std::cout << instance << std::endl;
		}

		HashSet<float> hashSet{10, GMEM};
		hashSet.Insert(48, 24);
		hashSet.Insert(12, 6);
		hashSet.Insert(22, 8);
		hashSet.RemoveAt(12);

		for (const auto& [index, instance] : hashSet)
		{
			std::cout << instance << " " << index << std::endl;
		}
	}

	return EXIT_SUCCESS;
}
