#include "pch.h"
#include "Components/Camera.h"
#include "Rendering/Renderer.h"

CameraSystem::CameraSystem(ce::Cecsar& cecsar, Renderer& renderer) : 
	SmallSystem<Camera>(cecsar, MAX_CAMERAS), 
	_renderer(renderer), _uboPool(renderer, SWAPCHAIN_MAX_FRAMES, MAX_CAMERAS)
{
	auto& swapChain = renderer.GetSwapChain();
	const uint32_t swapChainLength = swapChain.GetLength();

	vi::VkLayoutHandler::Info layoutInfo{};
	layoutInfo.bindings.Add(GetBindingInfo());
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	VkDescriptorType types = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uint32_t size = swapChainLength * MAX_CAMERAS;

	_descriptorPool.Construct(renderer, _layout, &types, &size, 1, size);
}

CameraSystem::~CameraSystem()
{
	_renderer.GetLayoutHandler().DestroyLayout(_layout);
	_descriptorPool.Cleanup();
}

void CameraSystem::Update()
{
	auto& shaderHandler = _renderer.GetShaderHandler();
	const uint32_t imageIndex = _renderer.GetSwapChain().GetImageIndex();

	uint32_t i = 0;
	for (auto& [index, camera] : *this)
	{
		shaderHandler.BindBuffer(camera._descriptors[imageIndex], camera._buffer, 0, sizeof(Camera::Ubo), 0, 0);
		i++;
	}
}

Camera& CameraSystem::Insert(const uint32_t sparseIndex, const Camera& value)
{
	auto& memoryHandler = _renderer.GetMemoryHandler();

	auto& camera = SmallSystem<Camera>::Insert(sparseIndex, value);
	camera._buffer = _uboPool.CreateBuffer();

	uint32_t i = 0;
	for (auto& descriptor : camera._descriptors)
	{
		descriptor = _descriptorPool.Get();
		memoryHandler.Bind(camera._buffer, _uboPool.GetMemory(), sizeof(Camera::Ubo) * i++);
	}
	return camera;
}

void CameraSystem::RemoveAt(const uint32_t index)
{
	auto& swapChainExt = _renderer.GetSwapChainExt();

	auto& camera = operator[](index);
	swapChainExt.Collect(camera._buffer);

	for (auto& descriptor : camera._descriptors)
		swapChainExt.Collect(descriptor, _descriptorPool);

	SmallSystem<Camera>::RemoveAt(index);
}

VkDescriptorSet CameraSystem::GetDescriptor(const Camera& value) const
{
	return value._descriptors[_renderer.GetSwapChain().GetImageIndex()];
}

vi::VkLayoutHandler::Info::Binding CameraSystem::GetBindingInfo()
{
	vi::VkLayoutHandler::Info::Binding camBinding{};
	camBinding.size = sizeof(Camera::Ubo);
	camBinding.flag = VK_SHADER_STAGE_VERTEX_BIT;
	return camBinding;
}
