#include "pch.h"
#include "Engine/Engine.h"

int main()
{
	Engine::Info info{};

	info.start = [](Engine& engine)
	{
		const auto camera = engine.GetCecsar().Add();
		engine.GetTransforms().Insert(camera);
		engine.GetCameras().Insert(camera);

		const auto quad = engine.GetCecsar().Add();
		engine.GetTransforms().Insert(quad);
		engine.GetMaterials().Insert(quad);

		const auto quad2 = engine.GetCecsar().Add();
		engine.GetTransforms().Insert(quad2);
		engine.GetMaterials().Insert(quad2);

		const auto quad3 = engine.GetCecsar().Add();
		auto& quad3Transform = engine.GetTransforms().Insert(quad3);
		engine.GetMaterials().Insert(quad3);
		quad3Transform.position = { 3, 0, 8 };
	};

	info.update = [](Engine& engine, bool& outQuit)
	{
		auto& c = engine.GetTransforms()[0];
		auto& t = engine.GetTransforms()[1];
		auto& t2 = engine.GetTransforms()[2];

		static float f = 0;
		f += 0.001f;
		//c.rotation = f * 360 / 8;
		c.position.y = sin(f) * 4;
		t.position.x = sin(f) * 5;
		t.position.y = cos(f) * 5;
		t.rotation = f * 30;
		t.position.z = 10;
		t2.position.z = 10 + sin(f) * -5;
	};

	vi::UniquePtr<Engine> engine{GMEM};
	return engine->Run(info);
}
