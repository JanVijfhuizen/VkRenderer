#include "pch.h"
#include "VkRenderer/VkCore/VkCore.h"
#include "VkRenderer/WindowHandlerGLFW.h"
#include "ECS/Cecsar.h"
#include "Components/Material.h"
#include "Components/Transform.h"

int main()
{
	{
		const uint32_t capacity = 100;

		vi::UniquePtr<vi::WindowHandlerGLFW> windowHandler{GMEM};
		vi::UniquePtr<vi::VkCore> core{};
		vi::UniquePtr<ce::Cecsar> cecsar{GMEM, capacity };

		{
			vi::VkCoreInfo info{};
			info.windowHandler = windowHandler;
			info.validationLayers.Add("VK_LAYER_RENDERDOC_Capture");
			core = { GMEM, info };
		}

		vi::UniquePtr<Transform::System> transforms{GMEM, *cecsar };
		vi::UniquePtr<Material::System> materials{GMEM, *cecsar, *core };

		auto& swapChain = core->GetSwapChain();
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
