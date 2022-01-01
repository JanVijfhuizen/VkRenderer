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

Light::System::System(const Info& info) : MapSet<Light>(8), _info(info)
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();

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
	uint32_t size = 8 * SWAPCHAIN_MAX_FRAMES;
	_descriptorPool = DescriptorPool(_layout, &uboType, &size, 1, size);

	vi::VkRenderer::LayoutInfo layoutInfoExt{};
	layoutInfoExt.bindings.push_back(lsm);
	vi::VkRenderer::LayoutInfo::Binding depthBuffer{};
	depthBuffer.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	depthBuffer.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutInfoExt.bindings.push_back(depthBuffer);
	_layoutExt = renderer.CreateLayout(layoutInfoExt);

	VkDescriptorType uboTypesExt[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };

	uint32_t sizeExt[] = {size, size};
	_descriptorPoolExt = DescriptorPool(_layoutExt, uboTypesExt, sizeExt, 2, size);
}

Light::System::~System()
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();

	for (int32_t i = GetCount() - 1; i >= 0; --i)
		EraseAt(i);

	renderer.DestroyCommandBuffer(_commandBuffer);
	renderer.DestroyFence(_fence);
	renderer.DestroyLayout(_layout);
	renderer.DestroyPipeline(_pipeline, _pipelineLayout);
	renderer.DestroyRenderPass(_renderPass);
	renderer.DestroyShaderModule(_vertModule);

	renderer.DestroyLayout(_layoutExt);
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

	for (auto& [sparseId, light] : *this)
	{
		auto& frame = light._frames[imageIndex];
		const auto& transform = transforms[sparseId];

		renderer.BeginRenderPass(frame.framebuffer, _renderPass, {}, _info.shadowResolution, &clearValue, 1);

		// Temp testing stuff.
		const glm::vec3 forward = Transform::System::GetForwardVector(transform.rotation);
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

		renderer.MapMemory(light._memory, &ubo, sizeof(Ubo) * imageIndex, sizeof(Ubo));
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

	light._buffer = renderer.CreateBuffer(sizeof(Ubo) * imageCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	light._memory = renderer.AllocateMemory(light._buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(light._buffer, light._memory, 0);

	for (uint32_t i = 0; i < imageCount; ++i)
	{
		auto& frame = light._frames[i];

		frame.descriptor = _descriptorPool.Get();
		renderer.BindBuffer(frame.descriptor, light._buffer, sizeof(Ubo) * i, sizeof(Ubo), 0, 0);

		CreateDepthBuffer(_info.shadowResolution, frame.image, frame.imageMemory, frame.imageView);
		frame.framebuffer = renderer.CreateFrameBuffer(&frame.imageView, 1, _renderPass,
			{
				static_cast<uint32_t>(_info.shadowResolution.x),
				static_cast<uint32_t>(_info.shadowResolution.y)
			});

		light._descriptorsExt[i] = _descriptorPoolExt.Get();
		frame.samplerExt = renderer.CreateSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, 
			VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
		renderer.BindBuffer(light._descriptorsExt[i], light._buffer, sizeof(Ubo) * i, sizeof(Ubo), 0, 0);
		renderer.BindSampler(light._descriptorsExt[i], frame.imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, frame.samplerExt, 1, 0);
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
		gc.Enqueue(frame.imageView);
		gc.Enqueue(frame.image);
		gc.Enqueue(frame.imageMemory);
		gc.Enqueue(frame.framebuffer);

		gc.Enqueue(light._descriptorsExt[i], _descriptorPoolExt);
		gc.Enqueue(frame.samplerExt);
	}
	gc.Enqueue(light._buffer);
	gc.Enqueue(light._memory);

	MapSet<Light>::EraseAt(index);
}

VkDescriptorSetLayout Light::System::GetLayout() const
{
	return _layoutExt;
}

void Light::System::CreateDepthBuffer(const glm::ivec2 resolution, 
	VkImage& outImage, VkDeviceMemory& outMemory, VkImageView& outImageView)
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();

	const auto format = renderer.GetDepthBufferFormat();

	outImage = renderer.CreateImage(resolution, format, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	outMemory = renderer.AllocateMemory(outImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	renderer.BindMemory(outImage, outMemory);
	outImageView = renderer.CreateImageView(outImage, format, VK_IMAGE_ASPECT_DEPTH_BIT);

	auto cmdBuffer = renderer.CreateCommandBuffer();
	const auto fence = renderer.CreateFence();

	renderer.BeginCommandBufferRecording(cmdBuffer);
	renderer.TransitionImageLayout(outImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

	renderer.EndCommandBufferRecording();
	renderer.Submit(&cmdBuffer, 1, nullptr, nullptr, fence);
	renderer.WaitForFence(fence);

	renderer.DestroyCommandBuffer(cmdBuffer);
	renderer.DestroyFence(fence);
}

const VkDescriptorSet* Light::GetDescriptors() const
{
	return _descriptorsExt;
}
