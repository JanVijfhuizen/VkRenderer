#include "pch.h"
#include "Transform.h"
#include "Rendering/Mesh.h"
#include "Engine.h"

int main()
{
	Engine::Info info{};
	info.start = []()
	{
		auto& cecsar = ce::Cecsar::Get();
		auto& meshSystem = Mesh::System::Get();
		auto& transformSystem = Transform::System::Get();

		const auto vertData = Vertex::Load("Cube.obj");
		const uint32_t handle = meshSystem.Create(vertData);

		const auto entity = cecsar.AddEntity();
		transformSystem.Insert(entity.index);
		auto& mesh = meshSystem.Insert(entity.index);
		mesh.handle = handle;
	};

	Engine::Run(info);

	return EXIT_SUCCESS;
}
