#include "pch.h"
#include "Components/Light.h"
#include "VkRenderer/VkHandlers/VkLayoutHandler.h"
#include "Rendering/Renderer.h"
#include "Components/Camera.h"
#include "Components/Material.h"
#include "Components/Transform.h"

LightSystem::LightSystem(ce::Cecsar& cecsar, Renderer& renderer, 
	CameraSystem& cameras, MaterialSystem& materials, 
	ShadowCasterSystem& shadowCasters, TransformSystem& transforms, const size_t size) :
	SmallSystem<Light>(cecsar, size), Dependency(renderer), 
	_cameras(cameras), _materials(materials), _shadowCasters(shadowCasters), _transforms(transforms)
{
	auto& swapChain = renderer.GetSwapChain();

	_shader = renderer.GetShaderExt().Load("light-");

	vi::VkLayoutHandler::CreateInfo layoutInfo{};
	auto& materialBinding = layoutInfo.bindings.Add();
	materialBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	materialBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	CreateMesh();

	VkDescriptorType types = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uint32_t sizes = 32 * swapChain.GetLength();

	_descriptorPool.Construct(renderer, _layout, &types, &sizes, 1, sizes);
	_shadowMaps = vi::ArrayPtr<ShadowMap>(size * swapChain.GetLength(), GMEM);

	OnRecreateSwapChainAssets();
}

LightSystem::~LightSystem()
{
	_descriptorPool.Cleanup();
	renderer.GetMeshHandler().Destroy(_mesh);
	renderer.GetShaderExt().DestroyShader(_shader);
	renderer.GetLayoutHandler().DestroyLayout(_layout);
	DestroySwapChainAssets();
}

void LightSystem::Draw()
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& meshHandler = renderer.GetMeshHandler();
	auto& pipelineHandler = renderer.GetPipelineHandler();
	auto& shaderHandler = renderer.GetShaderHandler();
	auto& swapChainExt = renderer.GetSwapChainExt();

	pipelineHandler.Bind(_pipeline, _pipelineLayout);
	meshHandler.Bind(_mesh);

	union
	{
		struct
		{
			VkDescriptorSet camera;
			VkDescriptorSet shadowCaster;
		};
		VkDescriptorSet values[2];
	} sets{};

	const auto fallbackTexture = _materials.GetFallbackTexture();
	glm::vec2 quadVertices[] = { glm::vec2(-1, -1), {-1, 1}, {1, 1}, {1, -1} };
	Ubo ubo{};

	for (auto& [camIndex, camera] : _cameras)
	{
		sets.camera = _cameras.GetDescriptor(camIndex);

		for (const auto& [lightIndex, light] : *this)
		{
			const auto& lightTransform = _transforms[lightIndex];
			const auto& lightPos = lightTransform.position;

			for (const auto& [shadowCasterIndex, shadowCaster] : _shadowCasters)
			{
				sets.shadowCaster = _descriptorPool.Get();

				assert(_materials.Contains(shadowCasterIndex));
				const auto& material = _materials[shadowCasterIndex];
				const auto& matTransform = _transforms[shadowCasterIndex];
				const auto& matPos = matTransform.position;

				const auto texture = material.texture ? material.texture : &fallbackTexture;
				const glm::vec2 offset = matPos - lightPos;
				const glm::vec2 offsetNorm = normalize(offset);

				vi::VkShaderHandler::SamplerCreateInfo samplerCreateInfo{};
				samplerCreateInfo.minLod = 0;
				samplerCreateInfo.maxLod = texture->mipLevels;
				const auto sampler = shaderHandler.CreateSampler(samplerCreateInfo);
				shaderHandler.BindSampler(sets.shadowCaster, texture->imageView, texture->layout, sampler, 0, 0);

				descriptorPoolHandler.BindSets(sets.values, sizeof sets / sizeof(VkDescriptorSet));

				// Calculate the shadow. We know for a fact that the mesh is a quad.
				ubo.height = offset.y;
				for (uint32_t i = 0; i < 4; ++i)
					ubo.vertices[i] = offset + vi::Ut::RotateRadians(quadVertices[i], atan2(offsetNorm.y, offsetNorm.x));

				shaderHandler.UpdatePushConstant(_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, ubo);
				meshHandler.Draw();

				swapChainExt.Collect(sampler);
				swapChainExt.Collect(sets.shadowCaster, _descriptorPool);
			}
		}
	}
}

VkVertexInputBindingDescription LightSystem::ShadowVertex::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(ShadowVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescription;
}

vi::Vector<VkVertexInputAttributeDescription> LightSystem::ShadowVertex::GetAttributeDescriptions()
{
	vi::Vector<VkVertexInputAttributeDescription> attributeDescriptions{ 2, GMEM_TEMP, 2 };

	auto& position = attributeDescriptions[0];
	position.binding = 0;
	position.location = 0;
	position.format = VK_FORMAT_R32_SINT;
	position.offset = offsetof(ShadowVertex, index);

	auto& texCoords = attributeDescriptions[1];
	texCoords.binding = 0;
	texCoords.location = 1;
	texCoords.format = VK_FORMAT_R32G32_SFLOAT;
	texCoords.offset = offsetof(ShadowVertex, textureCoordinates);

	return attributeDescriptions;
}

void LightSystem::CreateMesh()
{
	auto& meshHandler = renderer.GetMeshHandler();
	MeshHandler::VertexData<ShadowVertex> vertData{};
	vertData.vertices = vi::ArrayPtr<ShadowVertex>{ 4, GMEM_TEMP };
	
	Vertex::Index indices[6] = { 0, 1, 2, 0, 2, 3 };
	vertData.indices = vi::ArrayPtr<Vertex::Index>{ indices, 6 };

	auto& lBot = vertData.vertices[0];
	lBot.textureCoordinates = { 0, 0 };
	auto& lTop = vertData.vertices[1];
	lTop.textureCoordinates = { 0, 1 };
	auto& rTop = vertData.vertices[2];
	rTop.textureCoordinates = { 1, 1 };
	auto& rBot = vertData.vertices[3];
	rBot.textureCoordinates = { 1, 0 };

	for (uint32_t i = 0; i < 4; ++i)
		vertData.vertices[i].index = i;

	_mesh = meshHandler.Create(vertData);
}

void LightSystem::OnRecreateSwapChainAssets()
{
	if (_pipeline)
		DestroySwapChainAssets();

	auto& postEffectHandler = renderer.GetPostEffectHandler();

	vi::VkPipelineHandler::CreateInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = ShadowVertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = ShadowVertex::GetBindingDescription();
	pipelineInfo.setLayouts.Add(_cameras.GetLayout());
	pipelineInfo.setLayouts.Add(_layout);
	pipelineInfo.modules.Add(_shader.vertex);
	pipelineInfo.modules.Add(_shader.fragment);
	pipelineInfo.pushConstants.Add({ sizeof(Ubo), VK_SHADER_STAGE_VERTEX_BIT });
	pipelineInfo.renderPass = postEffectHandler.GetRenderPass();
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
