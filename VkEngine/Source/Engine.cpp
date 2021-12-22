#include "pch.h"
#include "Engine.h"
#include "Rendering/Mesh.h"
#include "Transform.h"
#include "Rendering/RenderManager.h"
#include "Rendering/Camera.h"
#include "VkRenderer/VkRenderer.h"
#include "Rendering/Texture.h"
#include "Rendering/SwapChainGC.h"

void Engine::Run(const Info& info)
{
	ce::Cecsar cecsar(info.capacity);

	{
		RenderManager renderManager{};

		{
			SwapChainGC gc;
			Texture::Manager textureManager;

			Transform::System transformSystem(info.capacity);
			Mesh::System meshSystem(info.capacity);
			Camera::System cameraSystem{};

			auto& swapChain = renderManager.GetVkRenderer().GetSwapChain();

			if(info.awake)
				info.awake();
			if(info.start)
				info.start();

			while (true)
			{
				bool quit = false;

				swapChain.WaitForImage();
				transformSystem.Update();

				if(info.preRenderUpdate)
				{
					info.preRenderUpdate(quit);
					if (quit)
						break;
				}

				renderManager.BeginFrame(quit, false);
				swapChain.BeginFrame(false);

				if (quit)
					break;

				cameraSystem.Update();

				if (info.update)
				{
					info.update(quit);
					if (quit)
						break;
				}

				renderManager.EndFrame();
			}

			renderManager.GetVkRenderer().DeviceWaitIdle();

			if (info.cleanup)
				info.cleanup();
		}
	}
}
