#include "pch.h"
#include "Components/Material.h"
#include "Rendering/Renderer.h"
#include "Components/Camera.h"
#include "Rendering/Vertex.h"
#include "Components/Transform.h"
#include "Rendering/MeshHandler.h"

MaterialSystem::MaterialSystem(ce::Cecsar& cecsar, 
	Renderer& renderer, TransformSystem& transforms, 
	CameraSystem& cameras, const char* shaderName) : 
	System<Material>(cecsar), Dependency(renderer), 
	_transforms(transforms), _cameras(cameras)
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& swapChain = renderer.GetSwapChain();
	const uint32_t swapChainLength = swapChain.GetLength();

	_shader = renderer.GetShaderExt().Load(shaderName);

	vi::VkLayoutHandler::CreateInfo layoutInfo{};
	auto& materialBinding = layoutInfo.bindings.Add();
	materialBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	materialBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	_mesh = renderer.GetMeshHandler().Create(MeshHandler::GenerateQuad());
	_fallbackTexture = renderer.GetTextureHandler().Create("Test", "png");

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
	DestroySwapChainAssets();
	renderer.GetLayoutHandler().DestroyLayout(_layout);
	renderer.GetShaderExt().DestroyShader(_shader);
	renderer.GetTextureHandler().Destroy(_fallbackTexture);
	renderer.GetMeshHandler().Destroy(_mesh);
	renderer.GetDescriptorPoolHandler().Destroy(_descriptorPool);
}

void MaterialSystem::OnRecreateSwapChainAssets()
{
	if (_pipeline)
		DestroySwapChainAssets();

	auto& postEffectHandler = renderer.GetPostEffectHandler();

	vi::VkPipelineHandler::CreateInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.Add(_cameras.GetLayout());
	pipelineInfo.setLayouts.Add(_layout);
	pipelineInfo.modules.Add(_shader.vertex);
	pipelineInfo.modules.Add(_shader.fragment);
	pipelineInfo.pushConstants.Add({ sizeof(Transform::PushConstant), VK_SHADER_STAGE_VERTEX_BIT });
	pipelineInfo.renderPass = postEffectHandler.GetRenderPass();
	pipelineInfo.extent = postEffectHandler.GetExtent();

	renderer.GetPipelineHandler().Create(pipelineInfo, _pipeline, _pipelineLayout);
}

void MaterialSystem::DestroySwapChainAssets() const
{
	renderer.GetPipelineHandler().Destroy(_pipeline, _pipelineLayout);
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
	auto& swapChainExt = renderer.GetSwapChainExt();

	pipelineHandler.Bind(_pipeline, _pipelineLayout);
	meshHandler.Bind(_mesh);

	const uint32_t startIndex = GetDescriptorStartIndex();

	union
	{
		struct
		{
			VkDescriptorSet camera;
			VkDescriptorSet material;
		};
		VkDescriptorSet values[2];
	} sets{};

	glm::mat4 modelMatrix;

	for (auto& [camIndex, camera] : _cameras)
	{
		sets.camera = _cameras.GetDescriptor(camIndex);

		for (const auto& [matIndex, material] : *this)
		{
			sets.material = _descriptorSets[startIndex + matIndex];

			const auto texture = material.texture ? material.texture : &_fallbackTexture;

			vi::VkShaderHandler::SamplerCreateInfo samplerCreateInfo{};
			samplerCreateInfo.minLod = 0;
			samplerCreateInfo.maxLod = texture->mipLevels;
			samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
			samplerCreateInfo.maxFilter = VK_FILTER_NEAREST;
			const auto sampler = shaderHandler.CreateSampler(samplerCreateInfo);
			shaderHandler.BindSampler(sets.material, texture->imageView, texture->layout, sampler, 0, 0);
			
			descriptorPoolHandler.BindSets(sets.values, sizeof sets / sizeof(VkDescriptorSet));

			const auto& transform = _transforms[matIndex];

			transform.CreateModelMatrix(modelMatrix);
			shaderHandler.UpdatePushConstant(_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, modelMatrix);
			meshHandler.Draw();

			swapChainExt.Collect(sampler);
		}
	}
}

Shader MaterialSystem::GetShader() const
{
	return _shader;
}

Mesh MaterialSystem::GetMesh() const
{
	return _mesh;
}

Texture MaterialSystem::GetFallbackTexture() const
{
	return _fallbackTexture;
}
