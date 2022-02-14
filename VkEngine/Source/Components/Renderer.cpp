#include "pch.h"
#include "Components/Renderer.h"
#include "Components/Material.h"
#include "Rendering/VulkanRenderer.h"
#include "Components/Camera.h"
#include "Rendering/Vertex.h"
#include "Components/Transform.h"
#include "Rendering/MeshHandler.h"
#include "Components/Light.h"
#include "VkRenderer/VkHandlers/VkDescriptorPoolHandler.h"
#include "VkRenderer/VkCore/VkCoreSwapchain.h"
#include "Rendering/PostEffectHandler.h"

RenderSystem::RenderSystem(ce::Cecsar& cecsar, VulkanRenderer& renderer, MaterialSystem& materials,
	CameraSystem& cameras, LightSystem& lights, TransformSystem& transforms, const char* shaderName) :
	System<Renderer>(cecsar), Dependency(renderer),
	_materials(materials), _cameras(cameras), _lights(lights), _transforms(transforms)
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& layoutHandler = renderer.GetLayoutHandler();
	auto& shaderExt = renderer.GetShaderExt();
	auto& swapChain = renderer.GetSwapChain();

	const uint32_t swapChainLength = swapChain.GetLength();

	_shader = shaderExt.Load(shaderName);

	// Create material layout.
	vi::VkLayoutHandler::CreateInfo layoutInfo{};
	auto& materialBinding = layoutInfo.bindings.Add();
	materialBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	materialBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	_layout = layoutHandler.CreateLayout(layoutInfo);

	_descriptorSets = vi::ArrayPtr<VkDescriptorSet>(swapChainLength * GetLength(), GMEM);

	VkDescriptorType types = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uint32_t size = GetLength() * swapChain.GetLength();

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

	OnRecreateSwapChainAssets();
}

RenderSystem::~RenderSystem()
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& layoutHandler = renderer.GetLayoutHandler();
	auto& shaderExt = renderer.GetShaderExt();

	DestroySwapChainAssets();

	layoutHandler.DestroyLayout(_layout);
	shaderExt.DestroyShader(_shader);
	descriptorPoolHandler.Destroy(_descriptorPool);
}

void RenderSystem::Draw()
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& meshHandler = renderer.GetMeshHandler();
	auto& pipelineHandler = renderer.GetPipelineHandler();
	auto& shaderHandler = renderer.GetShaderHandler();
	auto& swapChain = renderer.GetSwapChain();
	auto& swapChainExt = renderer.GetSwapChainExt();

	// Bind pipeline.
	pipelineHandler.Bind(_pipeline, _pipelineLayout);

	const uint32_t startIndex = GetDescriptorStartIndex();

	union
	{
		struct
		{
			VkDescriptorSet lighting;
			VkDescriptorSet camera;
			VkDescriptorSet material;
		};
		VkDescriptorSet values[3];
	} sets{};
	sets.lighting = _lights.GetDescriptorSet(swapChain.GetImageIndex());

	Mesh* mesh = nullptr;
	meshHandler.Bind(_materials.GetFallbackMesh());

	glm::mat4 modelMatrix;

	vi::VkShaderHandler::SamplerBindInfo bindInfo{};
	bindInfo.bindingIndex = 0;

	for (auto& [camIndex, camera] : _cameras)
	{
		sets.camera = _cameras.GetDescriptor(camIndex);

		for (const auto& [renderIndex, renderer] : *this)
		{
			auto& material = _materials[renderIndex];
			sets.material = _descriptorSets[startIndex + renderIndex];

			// Bind texture.
			Texture* texture = material.texture ? material.texture : &_materials.GetFallbackTexture();
			vi::VkShaderHandler::SamplerCreateInfo samplerCreateInfo{};
			samplerCreateInfo.minLod = 0;
			samplerCreateInfo.maxLod = texture->mipLevels;
			samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
			samplerCreateInfo.maxFilter = VK_FILTER_NEAREST;
			const auto sampler = shaderHandler.CreateSampler(samplerCreateInfo);

			bindInfo.set = sets.material;
			bindInfo.imageView = texture->imageView;
			bindInfo.layout = texture->layout;
			bindInfo.sampler = sampler;		
			shaderHandler.BindSampler(bindInfo);

			// Bind descriptor sets.
			descriptorPoolHandler.BindSets(sets.values, sizeof sets / sizeof(VkDescriptorSet));

			const auto& transform = _transforms[renderIndex];

			// Update the world transformation as a mat4x4.
			transform.CreateModelMatrix(modelMatrix);
			shaderHandler.UpdatePushConstant(_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, modelMatrix);

			// Bind and draw mesh.
			if (mesh != material.mesh)
			{
				mesh = material.mesh;
				meshHandler.Bind(mesh ? *mesh : _materials.GetFallbackMesh());
			}
			meshHandler.Draw();

			swapChainExt.Collect(sampler);
		}
	}
}

Shader& RenderSystem::GetShader()
{
	return _shader;
}

void RenderSystem::OnRecreateSwapChainAssets()
{
	if (_pipeline)
		DestroySwapChainAssets();

	auto& pipelineHandler = renderer.GetPipelineHandler();
	auto& postEffectHandler = renderer.GetPostEffectHandler();

	// This pipeline is assuming the usage of a model with standard vertices.
	vi::VkPipelineHandler::CreateInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.Add(_lights.GetLayout());
	pipelineInfo.setLayouts.Add(_cameras.GetLayout());
	pipelineInfo.setLayouts.Add(_layout);
	for (auto& module : _shader.modules)
		pipelineInfo.modules.Add(module);
	pipelineInfo.pushConstants.Add({ sizeof(glm::mat4), VK_SHADER_STAGE_VERTEX_BIT });

	// This assumes the state of the engine currently draws to the post effect handler, and not to the swap chain.
	pipelineInfo.renderPass = postEffectHandler.GetRenderPass();
	pipelineInfo.extent = postEffectHandler.GetExtent();

	pipelineHandler.Create(pipelineInfo, _pipeline, _pipelineLayout);
}

void RenderSystem::DestroySwapChainAssets() const
{
	auto& pipelineHandler = renderer.GetPipelineHandler();
	pipelineHandler.Destroy(_pipeline, _pipelineLayout);
}

uint32_t RenderSystem::GetDescriptorStartIndex() const
{
	auto& swapChain = renderer.GetSwapChain();
	const uint32_t imageIndex = swapChain.GetImageIndex();
	return GetLength() * imageIndex;
}
