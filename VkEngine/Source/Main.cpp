#include "pch.h"
#include "Transform.h"
#include "Rendering/Mesh.h"
#include "Engine.h"
#include "Rendering/Camera.h"
#include "Rendering/DefaultMaterial.h"

int main()
{
	Engine::Info info{};

	info.awake = []()
	{
		const uint32_t size = ce::Cecsar::Get().GetSize();
		GMEM.New<DefaultMaterial::System>(size);
	};

	info.start = []()
	{
		auto& cecsar = ce::Cecsar::Get();
		auto& meshSystem = Mesh::System::Get();
		auto& transformSystem = Transform::System::Get();
		auto& cameraSystem = Camera::System::Get();

		const auto vertData = Vertex::Load("Cube.obj");
		const uint32_t handle = meshSystem.Create(vertData);

		const auto cube = cecsar.AddEntity();
		transformSystem.Insert(cube.index);
		auto& mesh = meshSystem.Insert(cube.index);
		mesh.handle = handle;

		const auto camera = cecsar.AddEntity();
		auto& camComponent = cameraSystem.Insert(camera.index);
		auto& camTransform = transformSystem.Insert(camera.index);
		camTransform.position = { 0, 0, -10 };
	};

	info.update = [](bool& outQuit)
	{
		DefaultMaterial::System::Get().Update();
	};

	info.cleanup = []()
	{
		GMEM.Delete(&DefaultMaterial::System::Get());
	};

	Engine::Run(info);

	return EXIT_SUCCESS;
}
