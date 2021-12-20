#include "pch.h"
#include "Rendering/Camera.h"
#include "Rendering/RenderSystem.h"
#include "VkRenderer/VkRenderer.h"
#include "VkRenderer/WindowHandlerGLFW.h"
#include "Transform.h"

Camera::System::System() : MapSet<Camera>(1)
{
	auto& renderSystem = RenderManager::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	vi::VkRenderer::LayoutInfo camLayoutInfo{};
	vi::VkRenderer::LayoutInfo::Binding bindingInfo;
	bindingInfo.size = sizeof Ubo;
	bindingInfo.flag = VK_SHADER_STAGE_VERTEX_BIT;
	camLayoutInfo.bindings.push_back(bindingInfo);
	_layout = renderer.CreateLayout(camLayoutInfo);

	VkDescriptorType types = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uint32_t sizes = 1;

	_descriptorPool = DescriptorPool(_layout, &types, &sizes, 1, 1);
}

Camera::System::~System()
{
	auto& renderSystem = RenderManager::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	renderer.DestroyLayout(_layout);

	for (int32_t i = GetCount() - 1; i >= 0; --i)
		EraseAt(i);
}

void Camera::System::Update()
{
	auto& renderSystem = RenderManager::Get();
	auto& windowSystem = renderSystem.GetWindowHandler();
	auto& renderer = renderSystem.GetVkRenderer();

	auto& transformSystem = Transform::System::Get();

	const auto resolution = windowSystem.GetVkInfo().resolution;
	const float aspectRatio = static_cast<float>(resolution.x) / resolution.y;

	for (auto& [index, camera] : *this)
	{
		auto& transform = transformSystem[index];

		Ubo ubo{};
		ubo.view = glm::lookAt(transform.position, camera.lookat, glm::vec3(0, 1, 0));
		ubo.projection = glm::perspective(glm::radians(camera.fieldOfView),
			aspectRatio, camera.clipNear, camera.clipFar);

		renderer.MapMemory(camera._memory, &ubo, 0, sizeof(Ubo));
	}
}

KeyValuePair<unsigned, Camera>& Camera::System::Add(const KeyValuePair<unsigned, Camera>& value)
{
	auto& t = MapSet<Camera>::Add(value);
	auto& camera = t.value;

	auto& renderSystem = RenderManager::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	camera._buffer = renderer.CreateBuffer(sizeof(Ubo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	camera._memory = renderer.AllocateMemory(camera._buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(camera._buffer, camera._memory, 0);

	camera._descriptor = _descriptorPool.Get();
	renderer.BindBuffer(camera._descriptor, camera._buffer, 0, sizeof(Ubo), 0, 0);

	return t;
}

void Camera::System::EraseAt(const size_t index)
{
	auto& camera = operator[](index).value;

	auto& renderSystem = RenderManager::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	_descriptorPool.Add(camera._descriptor);

	renderer.FreeMemory(camera._memory);
	renderer.DestroyBuffer(camera._buffer);

	MapSet<Camera>::EraseAt(index);
}

Camera& Camera::System::GetMainCamera()
{
	return operator[](0).value;
}

VkDescriptorSetLayout Camera::System::GetLayout() const
{
	return _layout;
}

VkDescriptorSet Camera::GetDescriptor() const
{
	return _descriptor;
}
