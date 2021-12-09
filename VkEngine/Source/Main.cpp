#include "pch.h"
#include "Rendering/RenderSystem.h"
#include "Transform.h"
#include "Rendering/Mesh.h"
#include "VkRenderer/PoolAllocator.h"

int main()
{
	{
		const uint32_t capacity = 100;
		ce::Cecsar cecsar(capacity);
		RenderSystem renderSystem{};

		Transform::System transformSystem(capacity);
		Mesh::System meshSystem(capacity);

		const auto vertData = Vertex::Load("Cube.obj");
		const uint32_t handle = meshSystem.Create(vertData);

		while (true)
		{
			bool quit;
			renderSystem.BeginFrame(quit);	
			if (quit)
				break;

			renderSystem.EndFrame();
		}
	}

	return EXIT_SUCCESS;
}
