#include "pch.h"
#include "Transform.h"
#include "Rendering/Mesh.h"
#include "Engine.h"
#include "Rendering/Camera.h"

int main()
{
	Engine::Info info{};
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

	Engine::Run(info);

	return EXIT_SUCCESS;
}
