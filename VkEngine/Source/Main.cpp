#include "pch.h"
#include "VkRenderer/VkCore/VkCore.h"
#include "VkRenderer/WindowHandlerGLFW.h"

int main()
{
	{
		vi::WindowHandlerGLFW windowHandler;

		vi::VkCoreInfo info{};
		info.windowHandler = &windowHandler;

		vi::VkCore core{info};
	}

	return EXIT_SUCCESS;
}
