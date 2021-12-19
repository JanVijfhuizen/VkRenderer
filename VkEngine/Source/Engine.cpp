#include "pch.h"
#include "Engine.h"
#include "Rendering/Mesh.h"
#include "Transform.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Camera.h"

void Engine::Run(const Info& info)
{
	ce::Cecsar cecsar(info.capacity);

	{
		RenderSystem renderSystem{};

		{
			Transform::System transformSystem(info.capacity);
			Mesh::System meshSystem(info.capacity);
			Camera::System cameraSystem{};

			if(info.awake)
				info.awake();
			if(info.start)
				info.start();

			while (true)
			{
				bool quit;

				renderSystem.BeginFrame(quit);
				if (quit)
					break;

				cameraSystem.Update();

				if (info.update)
				{
					info.update(quit);
					if (quit)
						break;
				}

				renderSystem.EndFrame();
			}
		}
	}
}
