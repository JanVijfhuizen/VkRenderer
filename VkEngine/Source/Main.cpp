#include "pch.h"
#include "VkRenderer/WindowHandlerGLFW.h"
#include "VkRenderer/VkRenderer.h"

int main()
{
	vi::WindowHandlerGLFW windowHandler;

	vi::VkRenderer::Settings settings;
	settings.windowHandler = &windowHandler;
	settings.debugger.validationLayers.push_back("VK_LAYER_RENDERDOC_Capture");
	vi::VkRenderer renderer{settings};

	while (true)
	{
		bool quit = false;
		windowHandler.BeginFrame(quit);
		if (quit)
			break;
	}

	return 0;
}
