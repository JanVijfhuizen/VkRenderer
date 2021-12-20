#include "pch.h"
#include "Rendering/DefaultMaterial.h"
#include "Rendering/RenderSystem.h"
#include "FileReader.h"
#include "Rendering/Camera.h"
#include "VkRenderer/VkRenderer.h"
#include "Rendering/Vertex.h"
#include "Transform.h"
#include "Rendering/Mesh.h"

DefaultMaterial::System::System(const uint32_t size) : SparseSet<DefaultMaterial>(size)
{
	auto& renderSystem = RenderSystem::Get();
	auto& renderer = renderSystem.GetVkRenderer();
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
	auto& renderSystem = RenderSystem::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	renderer.DestroyPipeline(_pipeline, _pipelineLayout);
	renderer.DestroyShaderModule(_vertModule);
	renderer.DestroyShaderModule(_fragModule);
	renderer.DestroyLayout(_layout);
}

void DefaultMaterial::System::Update()
{
	auto& renderSystem = RenderSystem::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	auto& cameraSystem = Camera::System::Get();
	auto& transforms = Transform::System::Get();
	auto& meshes = Mesh::System::Get();

	if (cameraSystem.GetSize() == 0)
		return;

	const auto bakedTransforms = transforms.GetBakedTransforms();
	auto& mainCamera = cameraSystem.GetMainCamera();

	union
	{
		struct
		{
			VkDescriptorSet cameraSet;
			VkDescriptorSet materialSet;
		};
		VkDescriptorSet sets[2];
	};

	/*
	cameraSet = mainCamera.GetDescriptor();
	renderer.BindPipeline(_pipeline);

	for (const auto [material, sparseId] : *this)
	{
		const uint32_t denseId = GetDenseId(sparseId);
		auto& mesh = meshes[sparseId];
		auto& bakedTransform = bakedTransforms[transforms.GetDenseId(sparseId)];

		materialSet = descriptorSet;
		renderSystem.UseMesh(mesh);
		renderer.BindDescriptorSets(sets, 3);
		renderer.BindSampler(descriptorSet, diffuseTex.imageView, frame.matDiffuseSampler, 0, 0);
		renderer.UpdatePushConstant(_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, bakedTransform);
		renderer.Draw(mesh.indCount);
	}
	*/
}
