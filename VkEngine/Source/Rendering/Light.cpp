#include "pch.h"
#include "Rendering/Light.h"
#include "Rendering/RenderManager.h"
#include "VkRenderer/VkRenderer.h"
#include "FileReader.h"
#include "Rendering/Vertex.h"
#include "Transform.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/Mesh.h"
#include "Rendering/ShadowCaster.h"
#include "Rendering/SwapChainGC.h"

Light::System::System(const Info& info) : MapSet<Light>(info.maxLights), _info(info)
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();
	auto& swapChain = renderer.GetSwapChain();

	const auto vertCode = FileReader::Read("Shaders/shadowMapper.spv");
	_vertModule = renderer.CreateShaderModule(vertCode);

	vi::VkRenderer::LayoutInfo layoutInfo{};
	vi::VkRenderer::LayoutInfo::Binding lsm{};
	lsm.size = sizeof(Ubo);
	lsm.flag = VK_SHADER_STAGE_VERTEX_BIT;
	layoutInfo.bindings.push_back(lsm);
	_layout = renderer.CreateLayout(layoutInfo);

	vi::VkRenderer::RenderPassInfo renderPassInfo{};
	renderPassInfo.useColorAttachment = false;
	renderPassInfo.useDepthAttachment = true;
	renderPassInfo.depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	renderPassInfo.depthFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	_renderPass = renderer.CreateRenderPass(renderPassInfo);

	vi::VkRenderer::PipelineInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.push_back(_layout);

	pipelineInfo.modules.push_back(
		{
			_vertModule,
			VK_SHADER_STAGE_VERTEX_BIT
		});

	pipelineInfo.pushConstants.push_back(
		{
			sizeof Transform::Baked,
			VK_SHADER_STAGE_VERTEX_BIT
		});

	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.extent =
	{
		static_cast<uint32_t>(info.shadowResolution.x),
		static_cast<uint32_t>(info.shadowResolution.y)
	};
	pipelineInfo.depthBufferCompareOp = VK_COMPARE_OP_GREATER;

	renderer.CreatePipeline(pipelineInfo, _pipeline, _pipelineLayout);
	_commandBuffer = renderer.CreateCommandBuffer();
	_fence = renderer.CreateFence();

	VkDescriptorType uboType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uint32_t size = _info.maxLights * SWAPCHAIN_MAX_FRAMES;
	_descriptorPool = DescriptorPool(_layout, &uboType, &size, 1, size);

	vi::VkRenderer::LayoutInfo layoutInfoExt{};
	lsm.count = _info.maxLights;
	layoutInfoExt.bindings.push_back(lsm);
	vi::VkRenderer::LayoutInfo::Binding depthBuffer{};
	depthBuffer.count = _info.maxLights;
	depthBuffer.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	depthBuffer.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutInfoExt.bindings.push_back(depthBuffer);
	_layoutExt = renderer.CreateLayout(layoutInfoExt);

	VkDescriptorType uboTypesExt[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };

	uint32_t sizeExt[] = {SWAPCHAIN_MAX_FRAMES, SWAPCHAIN_MAX_FRAMES };
	_descriptorPoolExt = DescriptorPool(_layoutExt, uboTypesExt, sizeExt, 2, SWAPCHAIN_MAX_FRAMES);

	const uint32_t imageCount = swapChain.GetImageCount();
	_uboBuffer = renderer.CreateBuffer(sizeof(Ubo) * imageCount * _info.maxLights, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	_uboMemory = renderer.AllocateMemory(_uboBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(_uboBuffer, _uboMemory);

	_instances = reinterpret_cast<Instance*>(GMEM.MAlloc(sizeof(Instance) * size));
	for (uint32_t i = 0; i < size; ++i)
	{
		auto& instance = _instances[i];

		instance.sampler = renderer.CreateSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR,
			VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
		CreateDepthBuffer(info.shadowResolution, instance.depthBuffer);

		instance.frameBuffer = renderer.CreateFrameBuffer(&instance.depthBuffer.imageView, 1, _renderPass,
			{
				static_cast<uint32_t>(_info.shadowResolution.x),
				static_cast<uint32_t>(_info.shadowResolution.y)
			});
	}

	_frames = reinterpret_cast<Frame*>(GMEM.MAlloc(sizeof(Frame) * imageCount));
	for (uint32_t i = 0; i < SWAPCHAIN_MAX_FRAMES; ++i)
	{
		auto& frame = _frames[i];
		frame.descriptor = _descriptorPoolExt.Get();

		const uint32_t startIndex = i * _info.maxLights;

		for (uint32_t j = 0; j < info.maxLights; ++j)
		{
			const uint32_t index = startIndex + j;
			auto& instance = _instances[index];

			renderer.BindBuffer(frame.descriptor, _uboBuffer, sizeof(Ubo) * index, sizeof(Ubo), 0, j);
			renderer.BindSampler(frame.descriptor, instance.depthBuffer.imageView,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, instance.sampler, 1, j);
		}
	}
}

Light::System::~System()
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();
	auto& swapChain = renderer.GetSwapChain();

	for (int32_t i = GetCount() - 1; i >= 0; --i)
		EraseAt(i);

	renderer.DestroyCommandBuffer(_commandBuffer);
	renderer.DestroyFence(_fence);
	renderer.DestroyLayout(_layout);
	renderer.DestroyPipeline(_pipeline, _pipelineLayout);
	renderer.DestroyRenderPass(_renderPass);
	renderer.DestroyShaderModule(_vertModule);

	renderer.DestroyLayout(_layoutExt);

	renderer.DestroyBuffer(_uboBuffer);
	renderer.FreeMemory(_uboMemory);

	const uint32_t imageCount = _info.maxLights * swapChain.GetImageCount();
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		auto& instance = _instances[i];

		renderer.DestroySampler(instance.sampler);
		renderer.DestroyFrameBuffer(instance.frameBuffer);
		DestroyDepthBuffer(instance.depthBuffer);
	}

	GMEM.MFree(_instances);
	GMEM.MFree(_frames);
}

void Light::System::Update()
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();
	auto& swapChain = renderer.GetSwapChain();

	auto& shadowCasters = ShadowCaster::System::Get();
	auto& meshes = Mesh::System::Get();
	auto& transforms = Transform::System::Get();

	const auto bakedTransforms = transforms.GetBakedTransforms();
	const uint32_t imageIndex = swapChain.GetCurrentImageIndex();

	VkClearValue clearValue{};
	clearValue.depthStencil = { 0, 0 };

	renderer.BeginCommandBufferRecording(_commandBuffer);
	renderer.BindPipeline(_pipeline, _pipelineLayout);

	const glm::vec2 shape = {10, 10};
	const glm::mat4 projection = glm::ortho(
		-shape.x, shape.x, 
		-shape.y, shape.y, 
		_info.near, _info.far);

	uint32_t i = -1;
	const uint32_t startIndex = swapChain.GetCurrentImageIndex() * _info.maxLights;

	for (auto& [sparseId, light] : *this)
	{
		i++;
		if (i >= _info.maxLights)
			break;

		const uint32_t instanceIndex = i + startIndex;

		auto& instance = _instances[instanceIndex];
		auto& frame = light._frames[imageIndex];
		const auto& transform = transforms[sparseId];
		
		renderer.BeginRenderPass(instance.frameBuffer, _renderPass, {}, _info.shadowResolution, &clearValue, 1);

		const glm::vec3 forward = transform.GetForwardVector();
		const glm::mat4 view = glm::lookAt(transform.position, forward, glm::vec3(0, 1, 0));

		Ubo ubo{};
		ubo.lightSpaceMatrix = projection * view;

		switch (light.type)
		{
		case Type::directional:
			ubo.lightDir = forward;
			break;
		case Type::point: 
			break;
		default: 
			;
		}

		renderer.BindBuffer(frame.descriptor, _uboBuffer, sizeof(Ubo) * instanceIndex, sizeof(Ubo), 0, 0);

		renderer.MapMemory(_uboMemory, &ubo, sizeof(Ubo) * instanceIndex, sizeof(Ubo));
		renderer.BindDescriptorSets(&frame.descriptor, 1);

		for (const auto& [shadowCaster, shadowCasterSparseId] : shadowCasters)
		{
			auto& mesh = meshes[shadowCasterSparseId];
			const auto& meshData = meshes.GetData(mesh);
			const auto& bakedTransform = bakedTransforms[transforms.GetDenseId(shadowCasterSparseId)];

			renderer.BindVertexBuffer(meshData.vertexBuffer);
			renderer.BindIndicesBuffer(meshData.indexBuffer);

			renderer.UpdatePushConstant(_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, bakedTransform);
			renderer.Draw(meshData.indexCount);
		}

		renderer.EndRenderPass();
	}

	renderer.EndCommandBufferRecording();
	renderer.Submit(&_commandBuffer, 1, nullptr, nullptr, _fence);
	renderer.WaitForFence(_fence);
}

KeyValuePair<unsigned, Light>& Light::System::Add(const KeyValuePair<unsigned, Light>& keyPair)
{
	auto& t = MapSet<Light>::Add(keyPair);
	auto& light = t.value;

	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();
	auto& swapChain = renderer.GetSwapChain();

	const uint32_t imageCount = swapChain.GetImageCount();

	for (uint32_t i = 0; i < imageCount; ++i)
	{
		auto& frame = light._frames[i];
		frame.descriptor = _descriptorPool.Get();
	}

	return t;
}

void Light::System::EraseAt(const size_t index)
{
	auto& light = operator[](index).value;

	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();
	auto& swapChain = renderer.GetSwapChain();

	const uint32_t imageCount = swapChain.GetImageCount();

	auto& gc = SwapChainGC::Get();

	for (uint32_t i = 0; i < imageCount; ++i)
	{
		auto& frame = light._frames[i];
		gc.Enqueue(frame.descriptor, _descriptorPool);
	}

	MapSet<Light>::EraseAt(index);
}

VkDescriptorSetLayout Light::System::GetLayout() const
{
	return _layoutExt;
}

VkDescriptorSet Light::System::GetDescriptor() const
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();
	auto& swapChain = renderer.GetSwapChain();

	return _frames[swapChain.GetCurrentImageIndex()].descriptor;
}

void Light::System::CreateDepthBuffer(const glm::ivec2 resolution, DepthBuffer& outDepthBuffer)
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();

	const auto format = renderer.GetDepthBufferFormat();

	outDepthBuffer.image = renderer.CreateImage(resolution, format, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	outDepthBuffer.imageMemory = renderer.AllocateMemory(outDepthBuffer.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	renderer.BindMemory(outDepthBuffer.image, outDepthBuffer.imageMemory);
	outDepthBuffer.imageView = renderer.CreateImageView(outDepthBuffer.image, format, VK_IMAGE_ASPECT_DEPTH_BIT);

	auto cmdBuffer = renderer.CreateCommandBuffer();
	const auto fence = renderer.CreateFence();

	renderer.BeginCommandBufferRecording(cmdBuffer);
	renderer.TransitionImageLayout(outDepthBuffer.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

	renderer.EndCommandBufferRecording();
	renderer.Submit(&cmdBuffer, 1, nullptr, nullptr, fence);
	renderer.WaitForFence(fence);

	renderer.DestroyCommandBuffer(cmdBuffer);
	renderer.DestroyFence(fence);
}

void Light::System::DestroyDepthBuffer(DepthBuffer& depthBuffer)
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();

	renderer.DestroyImageView(depthBuffer.imageView);
	renderer.DestroyImage(depthBuffer.image);
	renderer.FreeMemory(depthBuffer.imageMemory);
}