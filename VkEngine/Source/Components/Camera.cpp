#include "pch.h"
#include "Components/Camera.h"
#include "Rendering/Renderer.h"

CameraSystem::CameraSystem(ce::Cecsar& cecsar, Renderer& renderer) : 
	SmallSystem<Camera>(cecsar, MAX_CAMERAS), _renderer(renderer)
{
	auto& swapChain = renderer.GetSwapChain();
	const uint32_t swapChainLength = swapChain.GetLength();

	vi::VkLayoutHandler::Info layoutInfo{};
	layoutInfo.bindings.Add(GetBindingInfo());
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	VkDescriptorType types = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uint32_t size = swapChainLength * MAX_CAMERAS;

	_descriptorPool.Construct(renderer, _layout, &types, &size, 1, size);

	auto& memoryHandler = renderer.GetMemoryHandler();
	auto& shaderHandler = renderer.GetShaderHandler();

	const auto tempBuffer = shaderHandler.CreateBuffer(sizeof(Camera::Ubo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	auto memRequirements = memoryHandler.GetRequirements(tempBuffer);
	memRequirements.size = sizeof(Camera::Ubo) * size;
	_memory = memoryHandler.Allocate(memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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
