#include "pch.h"
#include "Components/Camera.h"
#include "Rendering/Renderer.h"
#include "Components/Transform.h"

CameraSystem::CameraSystem(ce::Cecsar& cecsar, 
	Renderer& renderer, TransformSystem& transforms) : 
	SmallSystem<Camera>(cecsar, MAX_CAMERAS), 
	_renderer(renderer), _transforms(transforms),
	_uboPool(renderer, 1, MAX_CAMERAS * SWAPCHAIN_MAX_FRAMES)
{
	auto& swapChain = renderer.GetSwapChain();
	const uint32_t swapChainLength = swapChain.GetLength();

	vi::VkLayoutHandler::Info layoutInfo{};
	layoutInfo.bindings.Add(GetBindingInfo());
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	VkDescriptorType types = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uint32_t size = swapChainLength * MAX_CAMERAS;

	_descriptorPool.Construct(renderer, _layout, &types, &size, 1, size);
	_ubos = vi::ArrayPtr<Camera::Ubo>{MAX_CAMERAS, GMEM};
}

CameraSystem::~CameraSystem()
{
	_renderer.GetLayoutHandler().DestroyLayout(_layout);
	_descriptorPool.Cleanup();
}

void CameraSystem::Update()
{
	auto& memoryHandler = _renderer.GetMemoryHandler();
	auto& shaderHandler = _renderer.GetShaderHandler();
	auto& swapChainExt = _renderer.GetSwapChainExt();

	const uint32_t imageIndex = _renderer.GetSwapChain().GetImageIndex();
	const auto memory = _uboPool.GetMemory();

	const size_t memSize = sizeof(Camera::Ubo) * MAX_CAMERAS;
	const size_t memOffset = memSize * imageIndex;

	uint32_t i = 0;
	for (auto& [index, camera] : *this)
	{
		auto& transform = _transforms[index];
		
		// Update individual UBOs.
		auto& ubo = _ubos[i];
		ubo.position = transform.position;
		ubo.rotation = transform.rotation;
		ubo.clipFar = camera.clipFar;

		const auto buffer = _uboPool.CreateBuffer();
		auto& descriptor = camera._descriptors[imageIndex];

		memoryHandler.Bind(buffer, memory, memOffset + sizeof(Camera::Ubo) * i);
		shaderHandler.BindBuffer(descriptor, buffer, 0, sizeof(Camera::Ubo), 0, 0);

		swapChainExt.Collect(buffer);

		i++;
	}

	// Update all UBOs in one call.
	memoryHandler.Map(memory, _ubos.GetData(), memOffset, memSize);
}

Camera& CameraSystem::Insert(const uint32_t sparseIndex, const Camera& value)
{
	auto& camera = SmallSystem<Camera>::Insert(sparseIndex, value);
	for (auto& _descriptor : camera._descriptors)
		_descriptor = _descriptorPool.Get();
	return camera;
}

void CameraSystem::RemoveAt(const uint32_t index)
{
	auto& swapChainExt = _renderer.GetSwapChainExt();
	auto& camera = operator[](index);
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
