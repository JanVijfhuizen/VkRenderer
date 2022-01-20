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

	uint32_t sortableIndices[4];
	float sortableAngles[4];
	glm::vec2 sortableVertices[4];

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
				assert(_transforms.Contains(shadowCasterIndex));

				const auto& material = _materials[shadowCasterIndex];
				const auto& matTransform = _transforms[shadowCasterIndex];
				const auto& matPos = matTransform.position;

				const auto texture = material.texture ? material.texture : &fallbackTexture;
				const glm::vec3 offset = matPos - lightPos;
				const glm::vec2 offsetNorm = normalize(offset);

				vi::VkShaderHandler::SamplerCreateInfo samplerCreateInfo{};
				samplerCreateInfo.minLod = 0;
				samplerCreateInfo.maxLod = texture->mipLevels;
				const auto sampler = shaderHandler.CreateSampler(samplerCreateInfo);
				shaderHandler.BindSampler(sets.shadowCaster, texture->imageView, texture->layout, sampler, 0, 0);

				descriptorPoolHandler.BindSets(sets.values, sizeof sets / sizeof(VkDescriptorSet));

				// Calculate the shadow. We know for a fact that the mesh is a quad.
				ubo.height = offset.z;
				for (uint32_t i = 0; i < 4; ++i)
					ubo.vertices[i] = glm::vec2(offset.x, offset.y) + vi::Ut::RotateRadians(quadVertices[i], atan2(offsetNorm.y, offsetNorm.x));

				// Calculate the angle distance from the center of the quad to get the outer angles.
				const float centerAngle = atan2(offsetNorm.y, offsetNorm.x);
				for (uint32_t i = 0; i < 4; ++i)
				{
					const glm::vec2 vertPos = glm::vec2(offset) + vi::Ut::RotateDegrees(quadVertices[i], matTransform.rotation.z / 2);
					sortableVertices[i] = vertPos;
					sortableIndices[i] = i;
					const glm::vec2 vertDir = normalize(vertPos);
					// Calculate angle.
					const float vertAngle = atan2(vertDir.y, vertDir.x);
					// Calculate angle offset from center angle.
					sortableAngles[i] = atan2(sin(vertAngle - centerAngle), cos(vertAngle - centerAngle));
				}

				// Sort based on center offset.
				vi::Ut::LinSort(sortableIndices, sortableAngles, 0, 4);

				ubo.vertices[0] = sortableVertices[sortableIndices[0]];
				ubo.vertices[1] = sortableVertices[sortableIndices[3]];

				const glm::vec2 edge = glm::vec2(matPos) + offsetNorm * light.range;
				const glm::vec2 edgeDir = vi::Ut::RotateDegrees(offsetNorm, 90);

				// Tangent = Opposide / Adjecent
				// atan(sortableAngles[0]) = ? / length(ubo.vertices[0] - edge)
				// atan(sortableAngles[3]) * length(ubo.vertices[0] - edge) = ?
				const float l = atan(sortableAngles[0]) * length(ubo.vertices[0] - edge);
				const float l2 = atan(sortableAngles[3]) * length(ubo.vertices[1] - edge);

				ubo.vertices[2] = edge - edgeDir * l;
				ubo.vertices[3] = edge - edgeDir * l2;

				ubo.textureCoordinates[0] = { 0, 0 };
				ubo.textureCoordinates[1] = { 0, 1 };
				ubo.textureCoordinates[2] = { 1, 1 };
				ubo.textureCoordinates[3] = { 1, 0 };

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
	vi::Vector<VkVertexInputAttributeDescription> attributeDescriptions{ 1, GMEM_TEMP, 1 };

	auto& position = attributeDescriptions[0];
	position.binding = 0;
	position.location = 0;
	position.format = VK_FORMAT_R32_SINT;
	position.offset = offsetof(ShadowVertex, index);

	return attributeDescriptions;
}

void LightSystem::CreateMesh()
{
	auto& meshHandler = renderer.GetMeshHandler();
	MeshHandler::VertexData<ShadowVertex> vertData{};
	vertData.vertices = vi::ArrayPtr<ShadowVertex>{ 4, GMEM_TEMP };
	
	Vertex::Index indices[6] = { 0, 1, 2, 0, 2, 3 };
	vertData.indices = vi::ArrayPtr<Vertex::Index>{ indices, 6 };

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
