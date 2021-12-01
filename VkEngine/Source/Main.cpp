#include "pch.h"
#include "RenderSystem.h"
#include "DefaultAllocator.h"

int main()
{
	{
		DefaultAllocator defaultAllocator;
		Renderer::System renderSystem;

		while (true)
		{
			bool quit;
			renderSystem.BeginFrame(quit);	
			if (quit)
				break;

			renderSystem.EndFrame();
		}
	}

	return 0;
}
