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

		// Testing code.
		vi::VkRenderer::LayoutInfo layoutInfo{};
		const auto layout = renderer.CreateLayout(layoutInfo);

		VkDescriptorType uboTypes[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		uint32_t capacities[] = { 1, 1 };
		const auto pool = renderer.CreateDescriptorPool(uboTypes, capacities, 2);

		VkDescriptorSet set;
		renderer.CreateDescriptorSets(pool, layout, 1, &set);

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

		// Cleanup testing code.
		renderer.DestroyLayout(layout);
		renderer.DestroyDescriptorPool(pool);
	}

	return 0;
}
