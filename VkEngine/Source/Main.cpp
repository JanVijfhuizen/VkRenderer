#include "pch.h"
#include "RenderSystem.h"
#include "DefaultAllocator.h"
#include "VkRenderer/ArrayPtr.h"

int main()
{
	{
		DefaultAllocator defaultAllocator;

		float* f = defaultAllocator.Alloc<float>(17);
		ArrayPtr<float> array(f, 17);

		for (auto& f : array)
		{
			f = 123;
		}

		for (auto& f : array)
		{
			std::cout << f << std::endl;
		}
	}

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
