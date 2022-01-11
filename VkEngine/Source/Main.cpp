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
		engine.GetMaterials().Insert(quad);

		const auto quad2 = engine.GetCecsar().Insert(2);
		auto& quad2Transform = engine.GetTransforms().Insert(quad2);
		engine.GetMaterials().Insert(quad2);
	};

	info.update = [](Engine& engine, bool& outQuit)
	{
		auto& t = engine.GetTransforms()[1];
		auto& t2 = engine.GetTransforms()[2];

		static float f = 0;
		f += 0.001f;
		t.position.x = sin(f) * 5;
		t.position.y = cos(f) * 5;
		t.position.z = 10;
		t2.position.z = 10 + sin(f) * -5;
	};

	vi::UniquePtr<Engine> engine{GMEM};
	return engine->Run(info);
}
