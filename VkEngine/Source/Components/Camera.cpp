#include "pch.h"
#include "Components/Camera.h"
#include "Rendering/Renderer.h"
#include "Components/Transform.h"

CameraSystem::CameraSystem(ce::Cecsar& cecsar, 
	Renderer& renderer, TransformSystem& transforms, const uint32_t capacity) :
	SmallSystem<Camera>(cecsar, capacity),
	_renderer(renderer), _transforms(transforms),
	_uboPool(renderer, capacity, renderer.GetSwapChain().GetLength())
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& swapChain = renderer.GetSwapChain();
	const uint32_t swapChainLength = swapChain.GetLength();

	vi::VkLayoutHandler::CreateInfo layoutInfo{};
	layoutInfo.bindings.Add(GetBindingInfo());
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	VkDescriptorType types = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uint32_t size = swapChainLength * capacity;

	_ubos = vi::ArrayPtr<Camera::Ubo>{ capacity, GMEM };
	_descriptorSets = vi::ArrayPtr<VkDescriptorSet>(size, GMEM);

	vi::VkDescriptorPoolHandler::PoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.types = &types;
	descriptorPoolCreateInfo.capacities = &size;
	descriptorPoolCreateInfo.typeCount = 1;
	_descriptorPool = descriptorPoolHandler.Create(descriptorPoolCreateInfo);

	vi::VkDescriptorPoolHandler::SetCreateInfo descriptorSetCreateInfo{};
	descriptorSetCreateInfo.layout = _layout;
	descriptorSetCreateInfo.pool = _descriptorPool;
	descriptorSetCreateInfo.outSets = _descriptorSets.GetData();
	descriptorSetCreateInfo.setCount = size;
	descriptorPoolHandler.CreateSets(descriptorSetCreateInfo);
}

CameraSystem::~CameraSystem()
{
	_renderer.GetLayoutHandler().DestroyLayout(_layout);
	_renderer.GetDescriptorPoolHandler().Destroy(_descriptorPool);
}

void CameraSystem::Update()
{
	auto& memoryHandler = _renderer.GetMemoryHandler();
	auto& shaderHandler = _renderer.GetShaderHandler();
	auto& swapChain = _renderer.GetSwapChain();
	auto& swapChainExt = _renderer.GetSwapChainExt();

	const uint32_t imageIndex = swapChain.GetImageIndex();
	const uint32_t descriptorSetStartIndex = GetDescriptorStartIndex();

	const auto memory = _uboPool.GetMemory();

	const size_t memSize = sizeof(Camera::Ubo) * GetLength();
	const size_t memOffset = memSize * imageIndex;
	const float aspectRatio = static_cast<float>(swapChain.GetExtent().width) / swapChain.GetExtent().height;

	const auto buffer = _uboPool.CreateBuffer();
	memoryHandler.Bind(buffer, memory, memOffset);

	uint32_t i = 0;
	for (auto& [index, camera] : *this)
	{
		auto& transform = _transforms[index];
		
		// Update individual UBOs.
		auto& ubo = _ubos[i];
		ubo.position = transform.position;
		ubo.rotation = transform.rotation;
		ubo.clipFar = camera.clipFar;
		ubo.aspectRatio = aspectRatio;

		auto& descriptor = _descriptorSets[descriptorSetStartIndex + i];
		
		shaderHandler.BindBuffer(descriptor, buffer, sizeof(Camera::Ubo) * i, sizeof(Camera::Ubo), 0, 0);
		i++;
	}

	// Update all UBOs in one call.
	memoryHandler.Map(memory, _ubos.GetData(), memOffset, memSize);
	swapChainExt.Collect(buffer);
}

VkDescriptorSet CameraSystem::GetDescriptor(const uint32_t sparseIndex) const
{
	return _descriptorSets[GetDescriptorStartIndex() + sparseIndex];
}

VkDescriptorSetLayout CameraSystem::GetLayout() const
{
	return _layout;
}

vi::VkLayoutHandler::CreateInfo::Binding CameraSystem::GetBindingInfo()
{
	vi::VkLayoutHandler::CreateInfo::Binding camBinding{};
	camBinding.size = sizeof(Camera::Ubo);
	camBinding.flag = VK_SHADER_STAGE_VERTEX_BIT;
	return camBinding;
}

uint32_t CameraSystem::GetDescriptorStartIndex() const
{
	auto& swapChain = _renderer.GetSwapChain();
	const uint32_t imageIndex = swapChain.GetImageIndex();
	return GetLength() * imageIndex;
}
