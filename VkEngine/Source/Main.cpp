#include "pch.h"
#include "Transform.h"
#include "Rendering/Mesh.h"
#include "Engine.h"
#include "Rendering/Camera.h"
#include "Rendering/DefaultMaterial.h"
#include "Rendering/ShadowCaster.h"
#include "Rendering/Light.h"

int main()
{
	Engine::Info info{};

	info.awake = []()
	{
		const uint32_t size = ce::Cecsar::Get().GetSize();
		GMEM.New<DefaultMaterial::System>(size);
		GMEM.New<ShadowCaster::System>(size);
		GMEM.New<Light::System>();
	};

	info.start = []()
	{
		auto& cecsar = ce::Cecsar::Get();
		auto& meshSystem = Mesh::System::Get();
		auto& transformSystem = Transform::System::Get();
		auto& cameraSystem = Camera::System::Get();
		auto& defaultMaterialSystem = DefaultMaterial::System::Get();
		auto& shadowCasterSystem = ShadowCaster::System::Get();
		auto& lightSystem = Light::System::Get();

		const auto cube = cecsar.AddEntity();
		transformSystem.Insert(cube.index);
		defaultMaterialSystem.Insert(cube.index);
		meshSystem.Insert(cube.index);
		shadowCasterSystem.Insert(cube.index);

		const auto ground = cecsar.AddEntity();
		auto& groundTransform = transformSystem.Insert(ground.index);
		groundTransform.position.y = 5;
		groundTransform.scale = { 5, 1, 5 };
		defaultMaterialSystem.Insert(ground.index);
		meshSystem.Insert(ground.index);

		const auto camera = cecsar.AddEntity();
		cameraSystem.Insert(camera.index);
		auto& camTransform = transformSystem.Insert(camera.index);
		camTransform.position = { 0, 0, -10 };

		const auto light = cecsar.AddEntity();
		lightSystem.Insert(light.index);
		auto& lightTransform = transformSystem.Insert(light.index);
		lightTransform.position = { 20, 10, 5 };
	};

	info.update = [](bool& outQuit)
	{
		DefaultMaterial::System::Get().Update();
		auto& transformSystem = Transform::System::Get();

		static float f = 0;
		f += 0.001f;
		transformSystem[2].position = { sin(f) * 25, -5.f, cos(f) * 25 };
	};

	info.preRenderUpdate = [](bool& outQuit)
	{
		Light::System::Get().Update();
	};

	info.cleanup = []()
	{
		GMEM.Delete(&DefaultMaterial::System::Get());
		GMEM.Delete(&ShadowCaster::System::Get());
		GMEM.Delete(&Light::System::Get());
	};

	Engine::Run(info);

	return EXIT_SUCCESS;
}
