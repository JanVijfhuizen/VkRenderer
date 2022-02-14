#include "pch.h"
#include "Components/Material.h"
#include "Rendering/Renderer.h"
#include "Components/Camera.h"
#include "Rendering/Vertex.h"
#include "Components/Transform.h"
#include "Rendering/MeshHandler.h"
#include "Components/Light.h"
#include "VkRenderer/VkHandlers/VkDescriptorPoolHandler.h"
#include "VkRenderer/VkCore/VkCoreSwapchain.h"
#include "Rendering/PostEffectHandler.h"

MaterialSystem::MaterialSystem(ce::Cecsar& cecsar, 
	VulkanRenderer& renderer, CameraSystem& cameras, 
	LightSystem& lights, TransformSystem& transforms, const char* shaderName) :
	System<Material>(cecsar), Dependency(renderer), 
	_cameras(cameras), _lights(lights), _transforms(transforms)
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& layoutHandler = renderer.GetLayoutHandler();
	auto& meshHandler = renderer.GetMeshHandler();
	auto& shaderExt = renderer.GetShaderExt();
	auto& swapChain = renderer.GetSwapChain();
	auto& textureHandler = renderer.GetTextureHandler();

	const uint32_t swapChainLength = swapChain.GetLength();

	_shader = shaderExt.Load(shaderName);

	// Create material layout.
	vi::VkLayoutHandler::CreateInfo layoutInfo{};
	auto& materialBinding = layoutInfo.bindings.Add();
	materialBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	materialBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	_layout = layoutHandler.CreateLayout(layoutInfo);

	// Create fallback mesh and texture.
	_fallbackMesh = meshHandler.Create(MeshHandler::GenerateCube());
	_fallbackTexture = textureHandler.Create("Test", "png");

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

MaterialSystem::~MaterialSystem()
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& layoutHandler = renderer.GetLayoutHandler();
	auto& meshHandler = renderer.GetMeshHandler();
	auto& shaderExt = renderer.GetShaderExt();
	auto& textureHandler = renderer.GetTextureHandler();

	DestroySwapChainAssets();

	layoutHandler.DestroyLayout(_layout);
	shaderExt.DestroyShader(_shader);
	textureHandler.Destroy(_fallbackTexture);
	meshHandler.Destroy(_fallbackMesh);
	descriptorPoolHandler.Destroy(_descriptorPool);
}

void MaterialSystem::OnRecreateSwapChainAssets()
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

void MaterialSystem::DestroySwapChainAssets() const
{
	auto& pipelineHandler = renderer.GetPipelineHandler();
	pipelineHandler.Destroy(_pipeline, _pipelineLayout);
}

uint32_t MaterialSystem::GetDescriptorStartIndex() const
{
	auto& swapChain = renderer.GetSwapChain();
	const uint32_t imageIndex = swapChain.GetImageIndex();
	return GetLength() * imageIndex;
}

void MaterialSystem::Draw()
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

	Mesh* mesh = &_fallbackMesh;
	Texture* texture = &_fallbackTexture;
	glm::mat4 modelMatrix;

	for (auto& [camIndex, camera] : _cameras)
	{
		sets.camera = _cameras.GetDescriptor(camIndex);

		for (const auto& [matIndex, material] : *this)
		{
			sets.material = _descriptorSets[startIndex + matIndex];

			// Bind texture.
			texture = material.texture ? material.texture : &_fallbackTexture;
			vi::VkShaderHandler::SamplerCreateInfo samplerCreateInfo{};
			samplerCreateInfo.minLod = 0;
			samplerCreateInfo.maxLod = texture->mipLevels;
			samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
			samplerCreateInfo.maxFilter = VK_FILTER_NEAREST;
			const auto sampler = shaderHandler.CreateSampler(samplerCreateInfo);
			shaderHandler.BindSampler(sets.material, texture->imageView, texture->layout, sampler, 0, 0);

			// Bind descriptor sets.
			descriptorPoolHandler.BindSets(sets.values, sizeof sets / sizeof(VkDescriptorSet));

			const auto& transform = _transforms[matIndex];

			// Update the world transformation as a mat4x4.
			transform.CreateModelMatrix(modelMatrix);
			shaderHandler.UpdatePushConstant(_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, modelMatrix);

			// Bind and draw mesh.
			mesh = material.mesh ? material.mesh : &_fallbackMesh;
			meshHandler.Bind(*mesh);
			meshHandler.Draw();

			swapChainExt.Collect(sampler);
		}
	}
}

Shader& MaterialSystem::GetShader()
{
	return _shader;
}

Mesh& MaterialSystem::GetFallbackMesh()
{
	return _fallbackMesh;
}

Texture& MaterialSystem::GetFallbackTexture()
{
	return _fallbackTexture;
}
