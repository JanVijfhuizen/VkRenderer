#include "pch.h"
#include "Components/Light.h"
#include "Rendering/Renderer.h"
#include "Components/Material.h"
#include "Components/Transform.h"

LightSystem::LightSystem(ce::Cecsar& cecsar, Renderer& renderer,
	ShadowCasterSystem& shadowCasters, TransformSystem& transforms, const Info& info) :
	SmallSystem<Light>(cecsar, info.size), Dependency(renderer),
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
	CreateExtDescriptorDependencies();
}

LightSystem::~LightSystem()
{
	DestroySwapChainAssets();
	DestroyExtDescriptorDependencies();

	auto& commandBufferHandler = renderer.GetCommandBufferHandler();
	auto& layoutHandler = renderer.GetLayoutHandler();
	auto& syncHandler = renderer.GetSyncHandler();

	for (auto& frame : _frames)
	{
		commandBufferHandler.Destroy(frame.commandBuffer);
		syncHandler.DestroySemaphore(frame.signalSemaphore);
	}

	renderer.GetRenderPassHandler().Destroy(_renderPass);
	renderer.GetShaderExt().DestroyShader(_shader);
	layoutHandler.DestroyLayout(_layout);
	DestroyCubeMaps();	
	renderer.GetDescriptorPoolHandler().Destroy(_descriptorPool);
}

void LightSystem::Render(const VkSemaphore waitSemaphore, MaterialSystem& materials)
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

	_currentFragBuffer = _fragmentUboPool.CreateBuffer();
	memoryHandler.Bind(_currentFragBuffer, fragMemory, fragmentUboOffset);

	const float aspect = static_cast<float>(_shadowResolution.x) / _shadowResolution.y;
	const float near = 0.1f;
	glm::mat4 modelMatrix;

	auto mesh = materials.GetMesh();
	meshHandler.Bind(mesh);

	for (const auto& [lightIndex, light] : *this)
	{
		const auto& lightTransform = _transforms[lightIndex];
		const auto& position = lightTransform.position;

		// Update geometry ubo.
		auto& geomUbo = _geometryUbos[i];
		auto& shadowFaces = geomUbo.matrices;

		const glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, light.range);
		shadowFaces[0] = shadowProj * glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
		shadowFaces[1] = shadowProj * glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
		shadowFaces[2] = shadowProj * glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
		shadowFaces[3] = shadowProj * glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
		shadowFaces[4] = shadowProj * glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
		shadowFaces[5] = shadowProj * glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));

		// Update fragment ubo.
		auto& fragUbo = _fragmentUbos[i];
		fragUbo.position = lightTransform.position;
		fragUbo.range = light.range;
	}

	memoryHandler.Map(geomMemory, _geometryUbos.GetData(), geometryUboOffset, sizeof(GeometryUbo) * GetLength());
	memoryHandler.Map(fragMemory, _fragmentUbos.GetData(), fragmentUboOffset, sizeof(FragmentUbo) * GetLength());
	swapChainExt.Collect(geomBuffer);
	swapChainExt.Collect(_currentFragBuffer);

	for (const auto& [lightIndex, light] : *this)
	{
		auto& descriptorSet = _descriptorSets[offsetMultiplier + i];
		shaderHandler.BindBuffer(descriptorSet, geomBuffer, sizeof(GeometryUbo) * i, sizeof(GeometryUbo), 0, 0);
		shaderHandler.BindBuffer(descriptorSet, _currentFragBuffer, sizeof(FragmentUbo) * i, sizeof(FragmentUbo), 1, 0);
		descriptorPoolHandler.BindSets(&descriptorSet, 1);

		for (const auto& [matIndex, material] : materials)
		{
			const auto& transform = _transforms[matIndex];

			transform.CreateModelMatrix(modelMatrix);
			shaderHandler.UpdatePushConstant(_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, modelMatrix);

			meshHandler.Draw();
		}

		++i;
	}

	shaderHandler.BindBuffer(_extDescriptorSets[imageIndex], _currentFragBuffer, 0, sizeof(FragmentUbo) * i, 0, 0);

	renderPassHandler.End();

	vi::VkCommandBufferHandler::SubmitInfo submitInfo{};
	submitInfo.buffers = &frame.commandBuffer;
	submitInfo.waitSemaphore = waitSemaphore;
	submitInfo.signalSemaphore = frame.signalSemaphore;
	submitInfo.buffersCount = 1;

	commandBufferHandler.EndRecording();
	commandBufferHandler.Submit(submitInfo);
}

VkSemaphore LightSystem::GetRenderFinishedSemaphore() const
{
	auto& swapChain = renderer.GetSwapChain();
	return _frames[swapChain.GetImageIndex()].signalSemaphore;
}

VkDescriptorSetLayout LightSystem::GetLayout() const
{
	return _extLayout;
}

VkDescriptorSet LightSystem::GetDescriptorSet(const uint32_t index) const
{
	return _extDescriptorSets[index];
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

void LightSystem::CreateExtDescriptorDependencies()
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& shaderHandler = renderer.GetShaderHandler();
	auto& swapChain = renderer.GetSwapChain();

	vi::VkLayoutHandler::CreateInfo extLayoutInfo{};
	auto& extFragBinding = extLayoutInfo.bindings.Add();
	extFragBinding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	extFragBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	extFragBinding.size = sizeof(FragmentUbo);
	extFragBinding.count = 6;
	auto& cubeMapsBinding = extLayoutInfo.bindings.Add();
	cubeMapsBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	cubeMapsBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	cubeMapsBinding.count = 6;
	_extLayout = renderer.GetLayoutHandler().CreateLayout(extLayoutInfo);

	// Create descriptor sets.
	const uint32_t length = swapChain.GetLength();
	_extDescriptorSets = vi::ArrayPtr<VkDescriptorSet>(length, GMEM);

	VkDescriptorType types[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
	uint32_t sizes[] = { length, length };

	vi::VkDescriptorPoolHandler::PoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.types = types;
	descriptorPoolCreateInfo.capacities = sizes;
	descriptorPoolCreateInfo.typeCount = 2;
	_extDescriptorPool = descriptorPoolHandler.Create(descriptorPoolCreateInfo);

	vi::VkDescriptorPoolHandler::SetCreateInfo descriptorSetCreateInfo{};
	descriptorSetCreateInfo.layout = _extLayout;
	descriptorSetCreateInfo.pool = _extDescriptorPool;
	descriptorSetCreateInfo.outSets = _extDescriptorSets.GetData();
	descriptorSetCreateInfo.setCount = length;
	descriptorPoolHandler.CreateSets(descriptorSetCreateInfo);

	// Bind samplers to external descriptor set.
	const uint32_t samplerCount = GetLength() * length;
	_extSamplers.Reallocate(samplerCount, GMEM);
	for (auto& sampler : _extSamplers)
		sampler = shaderHandler.CreateSampler();

	const uint32_t lightCount = GetLength();
	for (uint32_t i = 0; i < length; ++i)
	{
		auto& descriptorSet = _extDescriptorSets[i];

		for (uint32_t j = 0; j < lightCount; ++j)
			shaderHandler.BindSampler(descriptorSet, 
				_frames[i].cubeMap.view, 
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
			    _extSamplers[j + i * lightCount], 1, j);
	}
}

void LightSystem::DestroyExtDescriptorDependencies() const
{
	auto& shaderHandler = renderer.GetShaderHandler();

	for (auto& sampler : _extSamplers)
		shaderHandler.DestroySampler(sampler);

	renderer.GetLayoutHandler().DestroyLayout(_extLayout);
	renderer.GetDescriptorPoolHandler().Destroy(_extDescriptorPool);
}

void LightSystem::OnRecreateSwapChainAssets()
{
	if (_pipeline)
		DestroySwapChainAssets();

	vi::VkPipelineHandler::CreateInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.pushConstants.Add({ sizeof(Transform::PushConstant), VK_SHADER_STAGE_VERTEX_BIT });
	pipelineInfo.modules.Add(_shader.vertex);
	pipelineInfo.modules.Add(_shader.geometry);
	pipelineInfo.modules.Add(_shader.fragment);
	pipelineInfo.setLayouts.Add(_layout);
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.extent = _shadowResolution;
	pipelineInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	
	renderer.GetPipelineHandler().Create(pipelineInfo, _pipeline, _pipelineLayout);
}

void LightSystem::DestroySwapChainAssets() const
{
	renderer.GetPipelineHandler().Destroy(_pipeline, _pipelineLayout);
}

ShadowCasterSystem::ShadowCasterSystem(ce::Cecsar& cecsar) : System<ShadowCaster>(cecsar)
{
}
