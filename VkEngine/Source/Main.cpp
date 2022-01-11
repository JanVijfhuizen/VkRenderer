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

		const auto quad = engine.GetCecsar().Insert(1);
		auto& quadTransform = engine.GetTransforms().Insert(quad);
		//quadTransform.position.z = -.5;
		engine.GetMaterials().Insert(quad);
	};

	info.update = [](Engine& engine, bool& outQuit)
	{
		auto& t = engine.GetTransforms()[1];

		static float f = 0;
		f += 0.001f;
		t.position.x = sin(f);
		t.position.z = cos(f);
	};

	vi::UniquePtr<Engine> engine{GMEM};
	return engine->Run(info);
}
