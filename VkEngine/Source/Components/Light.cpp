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
	_geometryUbos(info.size, GMEM),
	_fragmentUbos(info.size, GMEM),
	_frames(renderer.GetSwapChain().GetLength(), GMEM)
{
	auto& commandBufferHandler = renderer.GetCommandBufferHandler();
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& renderPassHandler = renderer.GetRenderPassHandler();
	auto& swapChain = renderer.GetSwapChain();
	auto& syncHandler = renderer.GetSyncHandler();

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
	descriptorSetCreateInfo.setCount = GetLength() * swapChainLength;
	descriptorPoolHandler.CreateSets(descriptorSetCreateInfo);

	for (auto& frame : _frames)
	{
		frame.commandBuffer = commandBufferHandler.Create();
		frame.signalSemaphore = syncHandler.CreateSemaphore();
	}

	OnRecreateSwapChainAssets();
}

LightSystem::~LightSystem()
{
	DestroySwapChainAssets();

	auto& commandBufferHandler = renderer.GetCommandBufferHandler();
	auto& syncHandler = renderer.GetSyncHandler();

	for (auto& frame : _frames)
	{
		commandBufferHandler.Destroy(frame.commandBuffer);
		syncHandler.DestroySemaphore(frame.signalSemaphore);
	}

	renderer.GetRenderPassHandler().Destroy(_renderPass);
	renderer.GetShaderExt().DestroyShader(_shader);
	renderer.GetLayoutHandler().DestroyLayout(_layout);
	DestroyCubeMaps();	
	renderer.GetDescriptorPoolHandler().Destroy(_descriptorPool);
}

void LightSystem::Render(const VkSemaphore waitSemaphore)
{
	auto& commandBufferHandler = renderer.GetCommandBufferHandler();
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& memoryHandler = renderer.GetMemoryHandler();
	auto& meshHandler = renderer.GetMeshHandler();
	auto& pipelineHandler = renderer.GetPipelineHandler();
	auto& renderPassHandler = renderer.GetRenderPassHandler();
	auto& shaderHandler = renderer.GetShaderHandler();
	auto& swapChain = renderer.GetSwapChain();
	auto& swapChainExt = renderer.GetSwapChainExt();

	const uint32_t imageIndex = swapChain.GetImageIndex();
	const uint32_t offsetMultiplier = GetLength() * imageIndex;

	auto& frame = _frames[imageIndex];
	commandBufferHandler.BeginRecording(frame.commandBuffer);

	// Begin render pass.
	VkClearValue depthStencil  = { 1, 0 };
	renderPassHandler.Begin(frame.cubeMap.frameBuffer, _renderPass, {}, _shadowResolution, &depthStencil, 1);
	pipelineHandler.Bind(_pipeline, _pipelineLayout);

	uint32_t i = 0;
	const size_t geometryUboOffset = sizeof(GeometryUbo) * offsetMultiplier;
	const size_t fragmentUboOffset = sizeof(FragmentUbo) * offsetMultiplier;

	const auto geomMemory = _geometryUboPool.GetMemory();
	const auto fragMemory = _fragmentUboPool.GetMemory();

	const auto geomBuffer = _geometryUboPool.CreateBuffer();
	memoryHandler.Bind(geomBuffer, geomMemory, geometryUboOffset);
	const auto fragBuffer = _fragmentUboPool.CreateBuffer();
	memoryHandler.Bind(fragBuffer, fragMemory, fragmentUboOffset);

	const float aspect = static_cast<float>(_shadowResolution.x) / _shadowResolution.y;
	const float near = 0.1f;
	glm::mat4 modelMatrix;

	auto mesh = _materials.GetMesh();
	meshHandler.Bind(mesh);

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

		for (const auto& [matIndex, material] : _materials)
		{
			const auto& transform = _transforms[matIndex];

			transform.CreateModelMatrix(modelMatrix);
			shaderHandler.UpdatePushConstant(_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, modelMatrix);

			meshHandler.Draw();
		}

		++i;
	}

	memoryHandler.Map(geomMemory, _geometryUbos.GetData(), geometryUboOffset, sizeof(GeometryUbo) * GetLength());
	memoryHandler.Map(fragMemory, _fragmentUbos.GetData(), fragmentUboOffset, sizeof(FragmentUbo) * GetLength());
	swapChainExt.Collect(geomBuffer);
	swapChainExt.Collect(fragBuffer);

	renderPassHandler.End();

	vi::VkCommandBufferHandler::SubmitInfo submitInfo{};
	submitInfo.buffers = &frame.commandBuffer;
	submitInfo.waitSemaphore = waitSemaphore;
	submitInfo.signalSemaphore = frame.signalSemaphore;
	submitInfo.buffersCount = 1;

	commandBufferHandler.EndRecording();
	commandBufferHandler.Submit(submitInfo);
}

VkSemaphore LightSystem::GetRenderFinishedSemaphore()
{
	auto& swapChain = renderer.GetSwapChain();
	return _frames[swapChain.GetImageIndex()].signalSemaphore;
}

void LightSystem::CreateCubeMaps(vi::VkCoreSwapchain& swapChain, const glm::ivec2 resolution)
{
	auto& commandBufferHandler = renderer.GetCommandBufferHandler();
	auto& frameBufferHandler = renderer.GetFrameBufferHandler();
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

	// Create frame buffer info.
	vi::VkFrameBufferHandler::CreateInfo frameBufferCreateInfo{};
	frameBufferCreateInfo.imageViewCount = 1;
	frameBufferCreateInfo.renderPass = _renderPass;
	frameBufferCreateInfo.extent = _shadowResolution;
	frameBufferCreateInfo.layerCount = 6;

	// Create transition info.
	vi::VkImageHandler::TransitionInfo transitionInfo{};
	transitionInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	transitionInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	transitionInfo.layerCount = 6;

	// Start transition.
	auto cmdBuffer = commandBufferHandler.Create();
	commandBufferHandler.BeginRecording(cmdBuffer);

	// Use the create infos to create a cubemap for every swapchain image.
	for (uint32_t i = 0; i < swapChain.GetLength(); ++i)
	{
		auto& cubeMap = _frames[i].cubeMap;
		cubeMap.image = imageHandler.Create(imageCreateInfo);
		cubeMap.memory = memoryHandler.Allocate(cubeMap.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		memoryHandler.Bind(cubeMap.image, cubeMap.memory);

		viewCreateInfo.image = cubeMap.image;
		cubeMap.view = imageHandler.CreateView(viewCreateInfo);

		transitionInfo.image = cubeMap.image;
		imageHandler.TransitionLayout(transitionInfo);

		frameBufferCreateInfo.imageViews = &cubeMap.view;
		cubeMap.frameBuffer = frameBufferHandler.Create(frameBufferCreateInfo);
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
	auto& frameBufferHandler = renderer.GetFrameBufferHandler();
	auto& imageHandler = renderer.GetImageHandler();
	auto& memoryHandler = renderer.GetMemoryHandler();

	for (auto& frame : _frames)
	{
		auto& cubeMap = frame.cubeMap;
		frameBufferHandler.Destroy(cubeMap.frameBuffer);
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
	pipelineInfo.setLayouts.Add(_layout);
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.extent = _shadowResolution;
	
	renderer.GetPipelineHandler().Create(pipelineInfo, _pipeline, _pipelineLayout);
}

void LightSystem::DestroySwapChainAssets() const
{
	renderer.GetPipelineHandler().Destroy(_pipeline, _pipelineLayout);
}

ShadowCasterSystem::ShadowCasterSystem(ce::Cecsar& cecsar) : System<ShadowCaster>(cecsar)
{
}
