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
	//info.useRenderDoc = true;

	info.start = [](Engine<GameState>& engine, GameState& gameState)
	{
		const auto camera = engine.GetCecsar().Add();
		auto& cameraTransform = engine.GetTransforms().Insert(camera);
		engine.GetCameras().Insert(camera);
		cameraTransform.position.z = -10;

		const auto ground = engine.GetCecsar().Add();
		auto& groundTransform = engine.GetTransforms().Insert(ground);
		engine.GetMaterials().Insert(ground);
		groundTransform.scale = glm::vec3(6);
		groundTransform.position.z = .1f;

		const auto quad1 = engine.GetCecsar().Add();
		engine.GetTransforms().Insert(quad1);
		engine.GetMaterials().Insert(quad1);

		const auto quad2 = engine.GetCecsar().Add();
		auto& quad3Transform = engine.GetTransforms().Insert(quad2);
		engine.GetMaterials().Insert(quad2);
		quad3Transform.position = { 3, 1, 0 };
	};

	info.update = [](Engine<GameState>& engine, GameState& gameState, bool& outQuit)
	{
		
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
