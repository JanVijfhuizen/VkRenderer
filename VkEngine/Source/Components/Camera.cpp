#include "pch.h"
#include "Components/Camera.h"
#include "Rendering/Renderer.h"

CameraSystem::CameraSystem(ce::Cecsar& cecsar, Renderer& renderer) : 
	SmallSystem<Camera>(cecsar, MAX_CAMERAS), _renderer(renderer)
{
	vi::VkLayoutHandler::Info layoutInfo{};
	layoutInfo.bindings.Add(GetBindingInfo());
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	VkDescriptorType types = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uint32_t size = SWAPCHAIN_MAX_FRAMES * MAX_CAMERAS;

	_descriptorPool.Construct(renderer, _layout, &types, &size, 1, size);
}

CameraSystem::~CameraSystem()
{
	_renderer.GetLayoutHandler().DestroyLayout(_layout);
	_descriptorPool.Cleanup();
}

vi::VkLayoutHandler::Info::Binding CameraSystem::GetBindingInfo()
{
	vi::VkLayoutHandler::Info::Binding camBinding{};
	camBinding.size = sizeof(Camera::Ubo);
	camBinding.flag = VK_SHADER_STAGE_VERTEX_BIT;
	return camBinding;
}
