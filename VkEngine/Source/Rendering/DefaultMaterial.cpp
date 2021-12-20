#include "pch.h"
#include "Rendering/DefaultMaterial.h"
#include "Rendering/RenderManager.h"
#include "FileReader.h"
#include "Rendering/Camera.h"
#include "VkRenderer/VkRenderer.h"
#include "Rendering/Vertex.h"
#include "Transform.h"
#include "Rendering/Mesh.h"
#include "Rendering/Texture.h"

DefaultMaterial::System::System(const uint32_t size) : SparseSet<DefaultMaterial>(size)
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();
	auto& swapChain = renderer.GetSwapChain();

	auto& cameraSystem = Camera::System::Get();

	const auto vertCode = FileReader::Read("Shaders/vert.spv");
	const auto fragCode = FileReader::Read("Shaders/frag.spv");

	_vertModule = renderer.CreateShaderModule(vertCode);
	_fragModule = renderer.CreateShaderModule(fragCode);

	vi::VkRenderer::LayoutInfo layoutInfo{};
	vi::VkRenderer::LayoutInfo::Binding diffuseBinding{};
	diffuseBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	diffuseBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutInfo.bindings.push_back(diffuseBinding);
	_layout = renderer.CreateLayout(layoutInfo);

	vi::VkRenderer::PipelineInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.push_back(cameraSystem.GetLayout());
	pipelineInfo.setLayouts.push_back(_layout);

	pipelineInfo.modules.push_back(
		{
			_vertModule,
			VK_SHADER_STAGE_VERTEX_BIT
		});
	pipelineInfo.modules.push_back(
		{
			_fragModule,
			VK_SHADER_STAGE_FRAGMENT_BIT
		});

	pipelineInfo.pushConstants.push_back(
		{
			sizeof Transform::Baked,
			VK_SHADER_STAGE_VERTEX_BIT
		});

	pipelineInfo.renderPass = swapChain.GetRenderPass();
	pipelineInfo.extent = swapChain.GetExtent();

	renderer.CreatePipeline(pipelineInfo, _pipeline, _pipelineLayout);
	VkDescriptorType uboTypes[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
	uint32_t sizes[] = { 32, 32 };
	_descriptorPool = DescriptorPool(_layout, uboTypes, sizes, 2, 32);
}

DefaultMaterial::System::~System()
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();

	for (const auto [defaultMaterial, sparseId]: *this)
		OnErase(sparseId);

	renderer.DestroyPipeline(_pipeline, _pipelineLayout);
	renderer.DestroyShaderModule(_vertModule);
	renderer.DestroyShaderModule(_fragModule);
	renderer.DestroyLayout(_layout);
}

void DefaultMaterial::System::Update()
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();
	auto& swapChain = renderer.GetSwapChain();

	auto& textureManager = Texture::Manager::Get();

	auto& cameraSystem = Camera::System::Get();
	auto& transformSystem = Transform::System::Get();
	auto& meshSystem = Mesh::System::Get();

	if (cameraSystem.GetSize() == 0)
		return;

	const uint32_t imageIndex = swapChain.GetCurrentImageIndex();
	const auto bakedTransforms = transformSystem.GetBakedTransforms();
	auto& mainCamera = cameraSystem.GetMainCamera();

	VkDescriptorSet sets[2]
	{
		mainCamera.GetDescriptors()[imageIndex]
	};

	renderer.BindPipeline(_pipeline, _pipelineLayout);

	for (const auto [material, sparseId] : *this)
	{
		const auto& mesh = meshSystem.GetData(meshSystem[sparseId]);
		const auto& bakedTransform = bakedTransforms[transformSystem.GetDenseId(sparseId)];
		const auto& diffuseTexture = textureManager.GetData(material.textureHandle);
		sets[1] = material._descriptor;

		renderer.BindVertexBuffer(mesh.vertexBuffer);
		renderer.BindIndicesBuffer(mesh.indexBuffer);

		renderer.BindDescriptorSets(sets, 2);
		renderer.BindSampler(material._descriptor, diffuseTexture.imageView, diffuseTexture.layout, material.diffuseSampler, 0, 0);
		renderer.UpdatePushConstant(_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, bakedTransform);
		renderer.Draw(mesh.indexCount);
	}
}

DefaultMaterial& DefaultMaterial::System::Insert(const uint32_t sparseId)
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();

	auto& defaultMaterial = SparseSet<DefaultMaterial>::Insert(sparseId);
	defaultMaterial.diffuseSampler = renderer.CreateSampler();
	defaultMaterial._descriptor = _descriptorPool.Get();
	return defaultMaterial;
}

void DefaultMaterial::System::Erase(const uint32_t sparseId)
{
	OnErase(sparseId);
	SparseSet<DefaultMaterial>::Erase(sparseId);
}

void DefaultMaterial::System::OnErase(const uint32_t sparseId)
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();

	auto& defaultMaterial = operator[](sparseId);
	renderer.DestroySampler(defaultMaterial.diffuseSampler);
	_descriptorPool.Add(defaultMaterial._descriptor);
}
