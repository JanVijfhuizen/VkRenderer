#include "pch.h"
#include "Components/Light.h"
#include "Rendering/VulkanRenderer.h"
#include "Components/Material.h"
#include "Components/Transform.h"
#include "VkRenderer/VkHandlers/VkSyncHandler.h"
#include "VkRenderer/VkHandlers/VkCommandBufferHandler.h"
#include "VkRenderer/VkHandlers/VkDescriptorPoolHandler.h"
#include "VkRenderer/VkHandlers/VkRenderPassHandler.h"
#include "VkRenderer/VkHandlers/VkLayoutHandler.h"
#include "VkRenderer/VkCore/VkCoreSwapchain.h"
#include "VkRenderer/VkHandlers/VkMemoryHandler.h"
#include "VkRenderer/VkHandlers/VkShaderHandler.h"
#include "VkRenderer/VkHandlers/VkImageHandler.h"
#include "VkRenderer/VkHandlers/VkFrameBufferHandler.h"

LightSystem::LightSystem(ce::Cecsar& cecsar, VulkanRenderer& renderer, MaterialSystem& materials,
	ShadowCasterSystem& shadowCasters, TransformSystem& transforms, const Info& info) :
	SmallSystem<Light>(cecsar, info.size), Dependency(renderer),
	_materials(materials), _shadowCasters(shadowCasters), _transforms(transforms),
	_shadowResolution(info.shadowResolution),
	_geometryUboAllocator(renderer, info.size, renderer.GetSwapChain().GetLength()),
	_fragmentUboAllocator(renderer, info.size + 1, renderer.GetSwapChain().GetLength()),
	_geometryUbos(info.size, GMEM),
	_fragmentUbos(info.size + 1, GMEM),
	_frames(renderer.GetSwapChain().GetLength(), GMEM)
{
	auto& commandBufferHandler = renderer.GetCommandBufferHandler();
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& layoutHandler = renderer.GetLayoutHandler();
	auto& renderPassHandler = renderer.GetRenderPassHandler();
	auto& shaderHandler = renderer.GetShaderExt();
	auto& swapChain = renderer.GetSwapChain();
	auto& syncHandler = renderer.GetSyncHandler();

	const uint32_t swapChainLength = swapChain.GetLength();

	ShaderExt::LoadInfo shaderLoadInfo{};
	shaderLoadInfo.geometry = true;
	_shader = shaderHandler.Load("light-", shaderLoadInfo);

	// Set up layout for rendering to the cubemaps.
	vi::VkLayoutHandler::CreateInfo layoutInfo{};
	auto& geomBinding = layoutInfo.bindings.Add();
	geomBinding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	geomBinding.flag = VK_SHADER_STAGE_GEOMETRY_BIT;
	geomBinding.size = sizeof(GeometryUbo);
	auto& fragBinding = layoutInfo.bindings.Add();
	fragBinding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	fragBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragBinding.size = sizeof(FragmentUbo);
	_layout = layoutHandler.CreateLayout(layoutInfo);

	vi::VkRenderPassHandler::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.useColorAttachment = false;
	renderPassCreateInfo.useDepthAttachment = true;
	renderPassCreateInfo.depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	renderPassCreateInfo.depthFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	_renderPass = renderPassHandler.Create(renderPassCreateInfo);

	// Create the image assets for the cubemaps.
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

	// Set up sync objects for GPU syncronization.
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
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& layoutHandler = renderer.GetLayoutHandler();
	auto& renderPassHandler = renderer.GetRenderPassHandler();
	auto& shaderHandler = renderer.GetShaderExt();
	auto& syncHandler = renderer.GetSyncHandler();

	for (auto& frame : _frames)
	{
		commandBufferHandler.Destroy(frame.commandBuffer);
		syncHandler.DestroySemaphore(frame.signalSemaphore);
	}

	renderPassHandler.Destroy(_renderPass);
	shaderHandler.DestroyShader(_shader);
	layoutHandler.DestroyLayout(_layout);
	DestroyCubeMaps();	
	descriptorPoolHandler.Destroy(_descriptorPool);
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
	VkClearValue depthStencil  = { 1.f, 0 };

	const size_t geometryUboOffset = sizeof(GeometryUbo) * offsetMultiplier;
	const size_t fragmentUboOffset = sizeof(FragmentUbo) * offsetMultiplier;

	const auto geomMemory = _geometryUboAllocator.GetMemory();
	const auto fragMemory = _fragmentUboAllocator.GetMemory();

	// Create frame bound buffers.
	const auto geomBuffer = _geometryUboAllocator.CreateBuffer();
	memoryHandler.Bind(geomBuffer, geomMemory, geometryUboOffset);
	const auto fragBuffer = _fragmentUboAllocator.CreateBuffer();
	memoryHandler.Bind(fragBuffer, fragMemory, fragmentUboOffset);

	const float aspect = static_cast<float>(_shadowResolution.x) / _shadowResolution.y;
	const float near = 0.1f;
	glm::mat4 modelMatrix;

	auto& mesh = _materials.GetFallbackMesh();
	meshHandler.Bind(mesh);

	// Forward all the lighting information to the UBO buffer arrays.
	uint32_t i = 0;
	for (const auto& [lightIndex, light] : *this)
	{
		const auto& lightTransform = _transforms[lightIndex];
		const auto& position = lightTransform.position;

		// Update geometry ubo.
		auto& geomUbo = _geometryUbos[i];
		auto& shadowFaces = geomUbo.matrices;

		const glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, light.range);

		// Define the view matrices for every cubemap face.
		shadowFaces[0] = glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
		shadowFaces[1] = glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
		shadowFaces[2] = glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, -1));
		shadowFaces[3] = glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
		shadowFaces[4] = glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
		shadowFaces[5] = glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));

		// Multiply view matrices by the camera projection.
		for (auto& face : shadowFaces)
			face = shadowProj * face;

		// Update fragment ubo.
		auto& fragUbo = _fragmentUbos[i];
		fragUbo.position = lightTransform.position;
		fragUbo.range = light.range;

		++i;
	}

	// Use the final index of the fragment ubos as the additional info struct.
	auto& additionalLightingInfo = _fragmentUbos[_fragmentUbos.GetLength() - 1];
	additionalLightingInfo.count = i;

	memoryHandler.Map(geomMemory, _geometryUbos.GetData(), geometryUboOffset, sizeof(GeometryUbo) * GetLength());
	memoryHandler.Map(fragMemory, _fragmentUbos.GetData(), fragmentUboOffset, sizeof(FragmentUbo) * (GetLength() + 1));
	swapChainExt.Collect(geomBuffer);
	swapChainExt.Collect(fragBuffer);

	vi::VkShaderHandler::BufferBindInfo geomBindInfo{};
	geomBindInfo.buffer = geomBuffer;
	geomBindInfo.range = sizeof(GeometryUbo);
	geomBindInfo.bindingIndex = 0;

	vi::VkShaderHandler::BufferBindInfo fragBindInfo{};
	fragBindInfo.buffer = fragBuffer;
	fragBindInfo.range = sizeof(FragmentUbo);
	fragBindInfo.bindingIndex = 1;

	// Actually start drawing the models based on the earlier calculated cubemap shadows.
	i = 0;
	for (const auto& [lightIndex, light] : *this)
	{
		auto& cubeMap = _cubeMaps[imageIndex * GetLength() + i];
		renderPassHandler.Begin(cubeMap.frameBuffer, _renderPass, {}, _shadowResolution, &depthStencil, 1);
		pipelineHandler.Bind(_pipeline, _pipelineLayout);

		auto& descriptorSet = _descriptorSets[offsetMultiplier + i];

		geomBindInfo.set = descriptorSet;
		geomBindInfo.offset = sizeof(GeometryUbo) * i;
		shaderHandler.BindBuffer(geomBindInfo);

		fragBindInfo.set = descriptorSet;
		fragBindInfo.offset = sizeof(FragmentUbo) * i;
		shaderHandler.BindBuffer(fragBindInfo);
		descriptorPoolHandler.BindSets(&descriptorSet, 1);

		// Draw everything that has a material, not taking into consideration the different renderers.
		for (const auto& [matIndex, material] : _materials)
		{
			const auto& transform = _transforms[matIndex];

			transform.CreateModelMatrix(modelMatrix);
			shaderHandler.UpdatePushConstant(_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, modelMatrix);

			meshHandler.Draw();
		}

		renderPassHandler.End();

		++i;
	}

	// Handle external descriptor set.
	const auto extDescriptorSet = GetDescriptorSet(imageIndex);

	geomBindInfo.set = extDescriptorSet;
	geomBindInfo.offset = 0;
	geomBindInfo.count = GetLength();
	shaderHandler.BindBuffer(geomBindInfo);

	fragBindInfo.set = extDescriptorSet;
	fragBindInfo.offset = sizeof(FragmentUbo) * GetLength();
	shaderHandler.BindBuffer(fragBindInfo);

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
	_cubeMaps.Reallocate(GetLength() * swapChain.GetLength(), GMEM);
	for (auto& cubeMap : _cubeMaps)
	{
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

	for (auto& cubeMap : _cubeMaps)
	{
		frameBufferHandler.Destroy(cubeMap.frameBuffer);
		imageHandler.DestroyView(cubeMap.view);
		imageHandler.Destroy(cubeMap.image);
		memoryHandler.Free(cubeMap.memory);
	}
}

void LightSystem::CreateExtDescriptorDependencies()
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& layoutHandler = renderer.GetLayoutHandler();
	auto& shaderHandler = renderer.GetShaderHandler();
	auto& swapChain = renderer.GetSwapChain();

	vi::VkLayoutHandler::CreateInfo extLayoutInfo{};
	auto& extFragBinding = extLayoutInfo.bindings.Add();
	extFragBinding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	extFragBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	extFragBinding.size = sizeof(FragmentUbo);
	extFragBinding.count = 6;
	auto& extFragAddInfoBinding = extLayoutInfo.bindings.Add();
	extFragAddInfoBinding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	extFragAddInfoBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	extFragAddInfoBinding.size = sizeof(FragmentUbo);
	auto& cubeMapsBinding = extLayoutInfo.bindings.Add();
	cubeMapsBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	cubeMapsBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	cubeMapsBinding.count = 6;
	_extLayout = layoutHandler.CreateLayout(extLayoutInfo);

	// Create descriptor sets.
	const uint32_t length = swapChain.GetLength();
	_extDescriptorSets.Reallocate(length, GMEM);

	VkDescriptorType types[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
	uint32_t sizes[] = { length * 2, length };

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

	// Already bind all the cubemap samplers to the descriptors, because it won't change.
	const uint32_t lightCount = GetLength();
	for (uint32_t i = 0; i < length; ++i)
	{
		auto& descriptorSet = _extDescriptorSets[i];

		for (uint32_t j = 0; j < lightCount; ++j)
		{
			const uint32_t index = j + i * lightCount;

			vi::VkShaderHandler::SamplerBindInfo bindInfo{};
			bindInfo.set = descriptorSet;
			bindInfo.imageView = _cubeMaps[index].view;
			bindInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			bindInfo.sampler = _extSamplers[index];
			bindInfo.bindingIndex = 2;
			bindInfo.arrayIndex = j;
			shaderHandler.BindSampler(bindInfo);
		}
	}
}

void LightSystem::DestroyExtDescriptorDependencies() const
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& layoutHandler = renderer.GetLayoutHandler();
	auto& shaderHandler = renderer.GetShaderHandler();

	for (auto& sampler : _extSamplers)
		shaderHandler.DestroySampler(sampler);

	layoutHandler.DestroyLayout(_extLayout);
	descriptorPoolHandler.Destroy(_extDescriptorPool);
}

void LightSystem::OnRecreateSwapChainAssets()
{
	auto& pipelineHandler = renderer.GetPipelineHandler();

	if (_pipeline)
		DestroySwapChainAssets();

	vi::VkPipelineHandler::CreateInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.pushConstants.Add({ sizeof(glm::mat4), VK_SHADER_STAGE_VERTEX_BIT });
	for (auto& module : _shader.modules)
		pipelineInfo.modules.Add(module);
	pipelineInfo.setLayouts.Add(_layout);
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.extent = _shadowResolution;
	pipelineInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	
	pipelineHandler.Create(pipelineInfo, _pipeline, _pipelineLayout);
}

void LightSystem::DestroySwapChainAssets() const
{
	auto& pipelineHandler = renderer.GetPipelineHandler();
	pipelineHandler.Destroy(_pipeline, _pipelineLayout);
}

ShadowCasterSystem::ShadowCasterSystem(ce::Cecsar& cecsar) : System<ShadowCaster>(cecsar)
{
}
