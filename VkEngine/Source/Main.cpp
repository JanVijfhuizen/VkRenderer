#include "pch.h"
#include "VkRenderer/WindowHandlerGLFW.h"

int main()
{
	const vi::WindowHandlerGLFW windowHandler;

	while (true)
	{
		bool quit = false;
		windowHandler.BeginFrame(quit);
		if (quit)
			break;
	}

	return 0;
}
