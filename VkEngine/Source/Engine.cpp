﻿#include "pch.h"
#include "Engine.h"
#include "Rendering/Mesh.h"
#include "Transform.h"
#include "Rendering/RenderSystem.h"

void Engine::Run(const Info& info)
{
	ce::Cecsar cecsar(info.capacity);

	{
		RenderSystem renderSystem{};

		{
			Transform::System transformSystem(info.capacity);
			Mesh::System meshSystem(info.capacity);

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
