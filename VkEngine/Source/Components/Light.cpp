#include "pch.h"
#include "Components/Light.h"
#include "Rendering/Renderer.h"
#include "Components/Material.h"
#include "Components/Transform.h"

LightSystem::LightSystem(ce::Cecsar& cecsar, Renderer& renderer, MaterialSystem& materials,
	ShadowCasterSystem& shadowCasters, TransformSystem& transforms, const Info& info) :
	SmallSystem<Light>(cecsar, info.size), Dependency(renderer),
	_materials(materials),
	_shadowCasters(shadowCasters), _transforms(transforms),
	_shadowResolution(info.shadowResolution),
	_geometryUboPool(renderer, info.size, renderer.GetSwapChain().GetLength()),
	_fragmentUboPool(renderer, info.size, renderer.GetSwapChain().GetLength()),
	_geometryUbos(info.size, GMEM_VOL),
	_fragmentUbos(info.size, GMEM_VOL)
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& renderPassHandler = renderer.GetRenderPassHandler();
	auto& shaderHandler = renderer.GetShaderHandler();
	auto& swapChain = renderer.GetSwapChain();

	const uint32_t swapChainLength = swapChain.GetLength();

	ShaderExt::LoadInfo shaderLoadInfo{};
	shaderLoadInfo.geometry = true;
	_shader = renderer.GetShaderExt().Load("light-", shaderLoadInfo);

	vi::VkLayoutHandler::CreateInfo layoutInfo{};
	auto& geomBinding = layoutInfo.bindings.Add();
	geomBinding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	geomBinding.flag = VK_SHADER_STAGE_GEOMETRY_BIT;
	geomBinding.size = sizeof(GeometryUbo);
	auto& fragBinding = layoutInfo.bindings.Add();
	fragBinding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	fragBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragBinding.size = sizeof(FragmentUbo);
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	vi::VkRenderPassHandler::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.useColorAttachment = false;
	renderPassCreateInfo.useDepthAttachment = true;
	renderPassCreateInfo.depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	renderPassCreateInfo.depthFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	_renderPass = renderPassHandler.Create(renderPassCreateInfo);

	CreateCubeMaps(swapChain, info.shadowResolution);
	OnRecreateSwapChainAssets();

	// Create descriptor sets.
	_descriptorSets = vi::ArrayPtr<VkDescriptorSet>(swapChain.GetLength() * GetLength(), GMEM);

	VkDescriptorType types = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uint32_t size = GetLength() * swapChainLength * 2;

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

LightSystem::~LightSystem()
{
	DestroySwapChainAssets();

	renderer.GetRenderPassHandler().Destroy(_renderPass);
	renderer.GetShaderExt().DestroyShader(_shader);
	renderer.GetLayoutHandler().DestroyLayout(_layout);
	DestroyCubeMaps();	
	renderer.GetDescriptorPoolHandler().Destroy(_descriptorPool);
}

void LightSystem::Draw()
{
	return;

	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& memoryHandler = renderer.GetMemoryHandler();
	auto& shaderHandler = renderer.GetShaderHandler();
	auto& swapChain = renderer.GetSwapChain();
	auto& swapChainExt = renderer.GetSwapChainExt();

	const uint32_t imageIndex = swapChain.GetImageIndex();
	const uint32_t offsetMultiplier = GetLength() * imageIndex;

	const float aspect = static_cast<float>(_shadowResolution.x) / _shadowResolution.y;
	const float near = 0.1f;

	uint32_t i = 0;
	const size_t geometryUboOffset = sizeof(GeometryUbo) * offsetMultiplier;
	const size_t fragmentUboOffset = sizeof(FragmentUbo) * offsetMultiplier;

	const auto geomMemory = _geometryUboPool.GetMemory();
	const auto fragMemory = _fragmentUboPool.GetMemory();

	const auto geomBuffer = _geometryUboPool.CreateBuffer();
	memoryHandler.Bind(geomBuffer, geomMemory, geometryUboOffset);
	const auto fragBuffer = _fragmentUboPool.CreateBuffer();
	memoryHandler.Bind(fragBuffer, fragMemory, fragmentUboOffset);

	for (const auto& [lightIndex, light] : *this)
	{
		auto& lightTransform = _transforms[lightIndex];

		// Update geometry ubo.
		auto& geomUbo = _geometryUbos[i];
		const glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, light.range);

		// Update fragment ubo.
		auto& fragUbo = _fragmentUbos[i];
		fragUbo.position = lightTransform.position;
		fragUbo.range = light.range;

		auto& descriptorSet = _descriptorSets[offsetMultiplier + i];
		shaderHandler.BindBuffer(descriptorSet, geomBuffer, sizeof(GeometryUbo) * i, sizeof(GeometryUbo), 0, 0);
		shaderHandler.BindBuffer(descriptorSet, fragBuffer, sizeof(FragmentUbo) * i, sizeof(FragmentUbo), 1, 0);
		descriptorPoolHandler.BindSets(&descriptorSet, 1);

		++i;
	}

	memoryHandler.Map(geomMemory, _geometryUbos.GetData(), geometryUboOffset, sizeof(GeometryUbo) * GetLength());
	memoryHandler.Map(fragMemory, _fragmentUbos.GetData(), fragmentUboOffset, sizeof(FragmentUbo) * GetLength());
	swapChainExt.Collect(geomBuffer);
	swapChainExt.Collect(fragBuffer);
}

void LightSystem::CreateCubeMaps(vi::VkCoreSwapchain& swapChain, const glm::ivec2 resolution)
{
	auto& commandBufferHandler = renderer.GetCommandBufferHandler();
	auto& imageHandler = renderer.GetImageHandler();
	auto& memoryHandler = renderer.GetMemoryHandler();
	auto& syncHandler = renderer.GetSyncHandler();

	// Create image with 6 sides.
	vi::VkImageHandler::CreateInfo imageCreateInfo{};
	imageCreateInfo.resolution = resolution;
	imageCreateInfo.arrayLayers = 6;
	imageCreateInfo.format = swapChain.GetDepthBufferFormat();
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	// Create image view.
	vi::VkImageHandler::ViewCreateInfo viewCreateInfo{};
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewCreateInfo.layerCount = 6;
	viewCreateInfo.format = swapChain.GetDepthBufferFormat();
	viewCreateInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

	// Create transition info.
	vi::VkImageHandler::TransitionInfo transitionInfo{};
	transitionInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	transitionInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	transitionInfo.layerCount = 6;

	// Start transition.
	auto cmdBuffer = commandBufferHandler.Create();
	commandBufferHandler.BeginRecording(cmdBuffer);

	// Use the create infos to create a cubemap for every swapchain image.
	_cubeMaps.Reallocate(swapChain.GetLength(), GMEM);
	for (uint32_t i = 0; i < swapChain.GetLength(); ++i)
	{
		auto& cubeMap = _cubeMaps[i];
		cubeMap.image = imageHandler.Create(imageCreateInfo);
		cubeMap.memory = memoryHandler.Allocate(cubeMap.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		memoryHandler.Bind(cubeMap.image, cubeMap.memory);

		viewCreateInfo.image = cubeMap.image;
		cubeMap.view = imageHandler.CreateView(viewCreateInfo);

		transitionInfo.image = cubeMap.image;
		imageHandler.TransitionLayout(transitionInfo);
	}

	const auto fence = syncHandler.CreateFence();

	vi::VkCommandBufferHandler::SubmitInfo submitInfo{};
	submitInfo.buffers = &cmdBuffer;
	submitInfo.buffersCount = 1;
	submitInfo.fence = fence;

	// End recording and execute transitions.
	commandBufferHandler.EndRecording();
	commandBufferHandler.Submit(submitInfo);
	syncHandler.WaitForFence(fence);

	// Destroy command buffer and fence.
	commandBufferHandler.Destroy(cmdBuffer);
	syncHandler.DestroyFence(fence);
}

void LightSystem::DestroyCubeMaps()
{
	auto& imageHandler = renderer.GetImageHandler();
	auto& memoryHandler = renderer.GetMemoryHandler();

	for (auto& cubeMap : _cubeMaps)
	{
		imageHandler.DestroyView(cubeMap.view);
		imageHandler.Destroy(cubeMap.image);
		memoryHandler.Free(cubeMap.memory);
	}
}

void LightSystem::OnRecreateSwapChainAssets()
{
	if (_pipeline)
		DestroySwapChainAssets();

	auto& postEffectHandler = renderer.GetPostEffectHandler();

	vi::VkPipelineHandler::CreateInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.pushConstants.Add({ sizeof(Transform::PushConstant), VK_SHADER_STAGE_VERTEX_BIT });
	pipelineInfo.modules.Add(_shader.vertex);
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.extent = postEffectHandler.GetExtent();

	renderer.GetPipelineHandler().Create(pipelineInfo, _pipeline, _pipelineLayout);
}

void LightSystem::DestroySwapChainAssets() const
{
	renderer.GetPipelineHandler().Destroy(_pipeline, _pipelineLayout);
}

ShadowCasterSystem::ShadowCasterSystem(ce::Cecsar& cecsar) : System<ShadowCaster>(cecsar)
{
}
