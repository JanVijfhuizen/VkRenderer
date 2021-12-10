#include "pch.h"
#include "Rendering/RenderSystem.h"
#include "Transform.h"
#include "Rendering/Mesh.h"
#include "VkRenderer/FreeListAllocator.h"

int main()
{
	{
		vi::FreeListAllocator alloc{512};
		int32_t* arr = reinterpret_cast<int32_t*>(alloc.Allocate(sizeof(int32_t) * 10));
		float* arrF = reinterpret_cast<float*>(alloc.Allocate(sizeof(float) * 32));

		for (int i = 0; i < 10; ++i)
		{
			arr[i] = i;
		}

		for (int i = 0; i < 32; ++i)
		{
			arrF[i] = i + 10;
		}

		for (int i = 0; i < 10; ++i)
		{
			std::cout << arr[i] << std::endl;
		}

		for (int i = 0; i < 32; ++i)
		{
			std::cout << arrF[i] << std::endl;
		}

		alloc.Free(arrF);

		return 0;
	}

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
