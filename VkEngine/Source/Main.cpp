#include "pch.h"
#include "VkRenderer/WindowHandlerGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "VkRenderer/SwapChain.h"

int main()
{
	{
		vi::WindowHandlerGLFW windowHandler;

		vi::VkRenderer::Settings settings;
		settings.windowHandler = &windowHandler;
		settings.debugger.validationLayers.push_back("VK_LAYER_RENDERDOC_Capture");
		vi::VkRenderer renderer{ settings };
		auto& swapChain = renderer.GetSwapChain();

		while (true)
		{
			bool quit;
			windowHandler.BeginFrame(quit);
			
			if (quit)
				break;

			swapChain.BeginFrame();

			// Do stuff.

			bool shouldRecreateAssets;
			swapChain.EndFrame(shouldRecreateAssets);
		}
	}

	return 0;
}
