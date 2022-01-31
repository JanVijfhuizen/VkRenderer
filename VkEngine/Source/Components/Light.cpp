#include "pch.h"
#include "Components/Light.h"
#include "VkRenderer/VkHandlers/VkLayoutHandler.h"
#include "Rendering/Renderer.h"
#include "Components/Camera.h"
#include "Components/Material.h"
#include "Components/Transform.h"
#include "Utils/EUtils.h"

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
	float sortableValues[4];
	VertData vertData[4];
	const uint32_t hatVertOrder[3]{ 2, 1, 3 };
	const glm::vec2 texCoords[4]{ {0, 0}, {0, 1}, {1, 1}, {1, 0} };

	for (auto& [camIndex, camera] : _cameras)
	{
		sets.camera = _cameras.GetDescriptor(camIndex);

		for (const auto& [lightIndex, light] : *this)
		{
			const auto& lightTransform = _transforms[lightIndex];
			const auto& lightPos = lightTransform.position;

			for (const auto& [shadowCasterIndex, shadowCaster] : _shadowCasters)
			{
				assert(_materials.Contains(shadowCasterIndex));
				assert(_transforms.Contains(shadowCasterIndex));

				const auto& material = _materials[shadowCasterIndex];
				const auto& matTransform = _transforms[shadowCasterIndex];
				const auto& matPos = matTransform.position;

				const auto texture = material.texture ? material.texture : &fallbackTexture;
				const glm::vec3 offset = matPos - lightPos;
				const glm::vec2 offsetNorm = normalize(offset);

				const float centerAngle = atan2(offsetNorm.y, offsetNorm.x);
				const glm::vec2 lightPos2d = glm::vec2(lightPos);

				// Precalculate all important information.
				for (uint32_t i = 0; i < 4; ++i)
				{
					sortableIndices[i] = i;
					auto& data = vertData[i];

					const glm::vec2 localPos = vi::Ut::RotateDegrees(quadVertices[i], matTransform.rotation.z / 2);
					// Calculate the vertex world position and distance to the light.
					data.worldPos = glm::vec2(offset) + localPos;
					data.disToLight = glm::distance(data.worldPos, lightPos2d);

					// Calculate angle to the light.
					const glm::vec2 vertDir = glm::normalize(data.worldPos);
					const float vertAngle = atan2(vertDir.y, vertDir.x);
					// Calculate angle difference.
					data.horAngleToLight = atan2(sin(vertAngle - centerAngle), cos(vertAngle - centerAngle));

					// Calculate angle to center of the quad.
					const glm::vec2 normDir = glm::normalize(localPos);
					const float normAngle = atan2(normDir.y, normDir.x);
					data.angleToQuad = normAngle;

					// Calculate height angle.
					data.vertAngleToLight = vi::Ut::Abs(data.disToLight / offset.z);
				}

				// Sort on distance to light.
				for (uint32_t i = 0; i < 4; ++i)
					sortableValues[i] = vertData[sortableIndices[i]].disToLight;
				vi::Ut::LinSort(sortableIndices, sortableValues, 0, 4);

				const float cAngle = vertData[sortableIndices[0]].horAngleToLight;
				const float aAngle = vertData[sortableIndices[1]].horAngleToLight;
				const float bAngle = vertData[sortableIndices[2]].horAngleToLight;

				// Based on the angle to the light, the shadow might have a different form.
				// A shadow with a "hat" is a shadow with a vertex in front that does not normally add to the shadow's shape.
				const bool hasHat = cAngle > aAngle && cAngle < bAngle || cAngle < aAngle&& cAngle > bAngle;

				// Sort on angle to light.
				for (uint32_t i = 1; i < 3; ++i)
					sortableValues[i] = -vertData[sortableIndices[i]].horAngleToLight;
				vi::Ut::LinSort(sortableIndices, sortableValues, 1, 3);

				if (hasHat)
				{
					// Draw the shadow front side.
					for (uint32_t i = 0; i < 3; ++i)
						ubo.vertices[i] = vertData[sortableIndices[i]].worldPos;

					// Draw the shadow back side.
					for (uint32_t i = 0; i < 3; ++i)
					{
						auto& data = vertData[sortableIndices[hatVertOrder[i]]];
						ubo.vertices[3 + i] = data.worldPos + glm::normalize(data.worldPos) * data.vertAngleToLight;
					}

					// Assign unsorted texture coordinates.
					ubo.textureCoordinates[0] = texCoords[0];
					ubo.textureCoordinates[4] = ubo.textureCoordinates[1] = texCoords[3];
					ubo.textureCoordinates[3] = ubo.textureCoordinates[2] = texCoords[1];
					ubo.textureCoordinates[5] = texCoords[2];
				}
				else
				{
					// Sort on angle to the quad.
					for (uint32_t i = 0; i < 4; ++i)
						sortableValues[i] = -vertData[sortableIndices[i]].angleToQuad;
					vi::Ut::LinSort(sortableIndices, sortableValues, 0, 4);

					// Sort on angle towards the quad center.
					for (uint32_t i = 0; i < 4; ++i)
						sortableValues[i] = -vertData[sortableIndices[i]].horAngleToLight;
					EUt::LinMove(sortableIndices, sortableValues, 0, 4);

					// Draw the shadow front side.
					for (uint32_t i = 0; i < 2; ++i)
					{
						auto& data = vertData[sortableIndices[i * 3]];
						ubo.vertices[i * 2] = data.worldPos;
						// Shadow end.
						ubo.vertices[i * 2 + 1] = data.worldPos + glm::normalize(data.worldPos) * data.vertAngleToLight;
					}

					// Draw the shadow back side.
					for (uint32_t i = 0; i < 2; ++i)
					{
						// Shadow end.
						auto& data = vertData[sortableIndices[i + 1]];
						ubo.vertices[i + 4] = data.worldPos + glm::normalize(data.worldPos) * data.vertAngleToLight;
					}
				}

				// Draw the quad shadow.
				ubo.height = offset.z;
				sets.shadowCaster = _descriptorPool.Get();
				vi::VkShaderHandler::SamplerCreateInfo samplerCreateInfo{};
				samplerCreateInfo.minLod = 0;
				samplerCreateInfo.maxLod = texture->mipLevels;
				const auto sampler = shaderHandler.CreateSampler(samplerCreateInfo);
				shaderHandler.BindSampler(sets.shadowCaster, texture->imageView, texture->layout, sampler, 0, 0);

				descriptorPoolHandler.BindSets(sets.values, sizeof sets / sizeof(VkDescriptorSet));

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
	vertData.vertices = vi::ArrayPtr<ShadowVertex>{ 6, GMEM_TEMP };

	Vertex::Index indices[12] = 
	{ 
		0, 1, 2, 
		2, 1, 3, 
		3, 1, 4,
		3, 4, 5
	};

	vertData.indices = vi::ArrayPtr<Vertex::Index>{ indices, 12 };

	for (uint32_t i = 0; i < 6; ++i)
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
