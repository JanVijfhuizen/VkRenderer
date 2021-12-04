#include "pch.h"
#include "Rendering/RenderSystem.h"
#include "Transform.h"

int main()
{
	{
		const uint32_t capacity = 100;
		ce::Cecsar cecsar(capacity);
		Renderer::System renderSystem(capacity);
		Transform::System transformSystem(capacity);

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
