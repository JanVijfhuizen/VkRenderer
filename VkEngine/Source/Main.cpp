#include "pch.h"
#include "Engine/Engine.h"

class GameState final
{
public:
	Texture texture;
};

typedef Engine<GameState> GameEngine;

int main()
{
	Engine<GameState>::Info info{};
	//info.useRenderDoc = true;

	info.start = [](Engine<GameState>& engine, GameState& gameState)
	{
		gameState.texture = engine.GetVulkanRenderer().GetTextureHandler().Create("Feather", "png");

		auto& cecsar = engine.GetCecsar();
		auto& cameras = engine.GetCameras();
		auto& lights = engine.GetLights();
		auto& materials = engine.GetMaterials();
		auto& renderers = engine.GetRenderers();
		auto& shadowCasters = engine.GetShadowCasters();
		auto& transforms = engine.GetTransforms();

		const auto camera = cecsar.Add();
		auto& cameraTransform = transforms.Insert(camera);
		cameras.Insert(camera);
		cameraTransform.position.z = -15;
		cameraTransform.position.y = 30;

		const auto ground = cecsar.Add();
		auto& groundTransform = transforms.Insert(ground);
		materials.Insert(ground);
		renderers.Insert(ground);
		groundTransform.scale = glm::vec3(150, 150, 1);
		groundTransform.position.z = 1;

		const auto quad1 = cecsar.Add();
		transforms.Insert(quad1);
		shadowCasters.Insert(quad1);
		auto& mat = materials.Insert(quad1);
		renderers.Insert(quad1);
		mat.texture = &gameState.texture;
		
		const auto quad2 = cecsar.Add();
		auto& quad3Transform = transforms.Insert(quad2);
		shadowCasters.Insert(quad2);
		materials.Insert(quad2);
		renderers.Insert(quad2);
		quad3Transform.position = { 3, 1, 0 };
		//quad3Transform.rotation.x = 360;
		
		const auto light = cecsar.Add();
		auto& lightTransform = transforms.Insert(light);
		//lightTransform.position.z -= 5;
		lightTransform.position.y -= 1;
		lights.Insert(light);

		const auto light2 = cecsar.Add();
		auto& light2Transform = transforms.Insert(light2);
		light2Transform.position.x -= 10;
		light2Transform.position.y -= 1;
		auto& l2 = lights.Insert(light2);
		cameraTransform.position.y += 15;
	};

	info.update = [](Engine<GameState>& engine, GameState& gameState, bool& outQuit)
	{
		static float f = .5f;
		f += 0.001f;

		//engine.GetTransforms()[0].position.z = -7.5f - 2.5f *sin(-f * 2);

		engine.GetTransforms()[2].position.x = sin(-f * 2) * 12;
		engine.GetTransforms()[2].position.y = cos(-f * 2) * 12;
		engine.GetTransforms()[3].rotation.z = f * 36;
		engine.GetTransforms()[3].position.x = sin(f) * 7;
		engine.GetTransforms()[3].position.y = cos(f) * 7;

		engine.GetTransforms()[4].position.z = (-1.f - sin(-f * 2)) * 2;
		//engine.GetTransforms()[4].position.z = 4;
	};

	info.cleanup = [](Engine<GameState>& engine, GameState& gameState)
	{
		engine.GetVulkanRenderer().GetTextureHandler().Destroy(gameState.texture);
	};

	auto engine = GMEM.New<Engine<GameState>>();
	const int result = engine->Run(info);
	GMEM.Delete(engine);
	return result;
}
