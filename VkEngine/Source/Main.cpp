#include "pch.h"
#include "VkRenderer/VkCore/VkCore.h"
#include "VkRenderer/WindowHandlerGLFW.h"
#include "Cecsar.h"
#include "SparseSet.h"

int main()
{
	{
		vi::WindowHandlerGLFW windowHandler;

		vi::VkCoreInfo info{};
		info.windowHandler = &windowHandler;

		vi::VkCore core{info};

		Cecsar cecsar{};
		SparseSet<float> sparseSet{100, GMEM};
		sparseSet.Insert(5, 10);
		sparseSet.Insert(15, 30);
		sparseSet.Insert(2, 4);
		sparseSet.RemoveAt(15);

		for (const auto& [instance, index] : sparseSet)
		{
			std::cout << instance << std::endl;
		}
	}

	return EXIT_SUCCESS;
}
