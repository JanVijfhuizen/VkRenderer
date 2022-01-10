#include "pch.h"
#include "VkRenderer/WindowHandlerGLFW.h"
#include "ECS/Cecsar.h"
#include "Components/Material.h"
#include "Components/Transform.h"
#include "Rendering/Renderer.h"

int main()
{
	{
		const uint32_t capacity = 100;

		const vi::UniquePtr<vi::WindowHandlerGLFW> windowHandler{GMEM};
		vi::UniquePtr<Renderer> renderer{};
		const vi::UniquePtr<ce::Cecsar> cecsar{GMEM, capacity };

		{
			vi::VkCoreInfo info{};
			info.windowHandler = windowHandler;
			info.validationLayers.Add("VK_LAYER_RENDERDOC_Capture");
			renderer = { GMEM, info };
		}

		vi::UniquePtr<Transform::System> transforms{GMEM, *cecsar };
		vi::UniquePtr<Material::System> materials{GMEM, *cecsar, *renderer, ""};

		auto& swapChain = renderer->GetSwapChain();
		while(true)
		{
			bool outQuit = false;
			windowHandler->BeginFrame(outQuit);
			if (outQuit)
				break;

			swapChain.BeginFrame();

			bool recreateAssets = false;
			swapChain.EndFrame(recreateAssets);
		}
	}

	return EXIT_SUCCESS;
}
