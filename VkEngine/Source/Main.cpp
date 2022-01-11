#include "pch.h"
#include "Engine/Engine.h"

int main()
{
	Engine::Info info{};
	info.start = [](Engine& engine)
	{
		const auto camera = engine.GetCecsar().Insert(0);
		engine.GetTransforms().Insert(camera);
		engine.GetCameras().Insert(camera);
	};

	vi::UniquePtr<Engine> engine{GMEM};
	return engine->Run(info);
}
