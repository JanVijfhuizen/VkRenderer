#include "pch.h"
#include "Engine/Engine.h"

class GameState final
{
public:
	BasicPostEffect* msaa;
};

typedef Engine<GameState> GameEngine;

int main()
{
	Engine<GameState>::Info info{};
	info.useRenderDoc = true;

	info.start = [](Engine<GameState>& engine, GameState& gameState)
	{
		auto& cecsar = engine.GetCecsar();
		auto& cameras = engine.GetCameras();
		auto& lights = engine.GetLights();
		auto& materials = engine.GetMaterials();
		auto& shadowCasters = engine.GetShadowCasters();
		auto& transforms = engine.GetTransforms();

		const auto camera = cecsar.Add();
		auto& cameraTransform = transforms.Insert(camera);
		cameras.Insert(camera);
		cameraTransform.position.z = -10;

		const auto ground = cecsar.Add();
		auto& groundTransform = transforms.Insert(ground);
		materials.Insert(ground);
		groundTransform.scale = glm::vec3(6);
		groundTransform.position.z = 0.1f;

		const auto quad1 = cecsar.Add();
		transforms.Insert(quad1);
		shadowCasters.Insert(quad1);
		materials.Insert(quad1);
		
		const auto quad2 = cecsar.Add();
		auto& quad3Transform = transforms.Insert(quad2);
		shadowCasters.Insert(quad2);
		materials.Insert(quad2);
		quad3Transform.position = { 3, 1, -1 };
		quad3Transform.rotation.x = 180;
		
		const auto light = cecsar.Add();
		auto& lightTransform = transforms.Insert(light);
		lightTransform.position.z += 1;
		lights.Insert(light);

		cameraTransform.position.y += 15;
	};

	info.update = [](Engine<GameState>& engine, GameState& gameState, bool& outQuit)
	{
		static float f = 0;
		f += 0.001f;

		engine.GetTransforms()[0].position.z = -7.5f - 2.5f *sin(-f * 2);

		engine.GetTransforms()[2].position.x = sin(-f * 2);
		engine.GetTransforms()[2].position.y = cos(-f * 2);
		//engine.GetTransforms()[3].rotation.z = f * 36;
		engine.GetTransforms()[3].position.x = sin(f) * 5;
		engine.GetTransforms()[3].position.y = cos(f) * 5;
	};

	info.cleanup = [](Engine<GameState>& engine, GameState& gameState)
	{
		//GMEM.Delete(gameState.msaa);
	};

	auto engine = GMEM.New<Engine<GameState>>();
	const int result = engine->Run(info);
	GMEM.Delete(engine);
	return result;
}
