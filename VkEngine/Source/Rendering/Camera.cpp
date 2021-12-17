#include "pch.h"
#include "Rendering/Camera.h"
#include "Rendering/RenderSystem.h"
#include "VkRenderer/VkRenderer.h"
#include "VkRenderer/WindowHandlerGLFW.h"
#include "Transform.h"

Camera::System::System() : MapSet<Camera>(1)
{

}

void Camera::System::Update()
{
	auto& renderSystem = RenderSystem::Get();
	auto& windowSystem = renderSystem.GetWindowHandler();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& swapChain = renderer.GetSwapChain();

	auto& transformSystem = Transform::System::Get();

	const auto resolution = windowSystem.GetVkInfo().resolution;
	const float aspectRatio = static_cast<float>(resolution.x) / resolution.y;
	const uint32_t imageIndex = swapChain.GetCurrentImageIndex();

	for (auto& [index, camera] : *this)
	{
		auto& transform = transformSystem[index];

		Ubo ubo{};
		ubo.view = glm::lookAt(transform.position, camera.lookat, glm::vec3(0, 1, 0));
		ubo.projection = glm::perspective(glm::radians(camera.fieldOfView),
			aspectRatio, camera.clipNear, camera.clipFar);

		renderer.MapMemory(camera._memory, &ubo, sizeof(Ubo) * imageIndex, 1);
	}
}

KeyValuePair<unsigned, Camera>& Camera::System::Add(const KeyValuePair<unsigned, Camera>& value)
{
	auto& t = MapSet<Camera>::Add(value);
	auto& camera = t.value;

	auto& renderSystem = RenderSystem::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& swapChain = renderer.GetSwapChain();

	const uint32_t imageCount = swapChain.GetImageCount();

	// Todo add descriptor from set.

	camera._buffer = renderer.CreateBuffer(sizeof(Ubo) * imageCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	camera._memory = renderer.AllocateMemory(camera._buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	renderer.BindMemory(camera._buffer, camera._memory);
	renderer.BindBuffer(camera._descriptor, camera._buffer, 0, sizeof(Ubo) * imageCount, 0, 0);

	return t;
}

void Camera::System::EraseAt(const size_t index)
{
	auto& camera = operator[](index).value;

	auto& renderSystem = RenderSystem::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	// Todo remove descriptor from set.

	renderer.FreeMemory(camera._memory);
	renderer.DestroyBuffer(camera._buffer);

	MapSet<Camera>::EraseAt(index);
}
