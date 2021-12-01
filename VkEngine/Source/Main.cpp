#include "pch.h"
#include "RenderSystem.h"

int main()
{
	{
		RenderSystem renderSystem;

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
