#include "pch.h"
#include "Components/Material.h"
#include "Rendering/Renderer.h"
#include "Components/Camera.h"
#include "Rendering/Vertex.h"
#include "Components/Transform.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/MeshHandler.h"

MaterialSystem::MaterialSystem(ce::Cecsar& cecsar, 
	Renderer& renderer, TransformSystem& transforms, const char* shaderName) : 
	System<Material>(cecsar), _renderer(renderer), _transforms(transforms)
{
	auto& swapChain = renderer.GetSwapChain();
	_shader = renderer.GetShaderExt().Load(shaderName);

	vi::VkLayoutHandler::Info layoutInfo{};
	vi::VkLayoutHandler::Info::Binding camBinding{};
	camBinding.size = sizeof(Camera::Ubo);
	camBinding.flag = VK_SHADER_STAGE_VERTEX_BIT;
	layoutInfo.bindings.Add(camBinding);
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	vi::VkPipelineHandler::Info pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.Add(_layout);
	pipelineInfo.modules.Add(_shader.vertex);
	pipelineInfo.modules.Add(_shader.fragment);
	pipelineInfo.pushConstants.Add({ sizeof(Transform), VK_SHADER_STAGE_VERTEX_BIT });
	pipelineInfo.renderPass = swapChain.GetRenderPass();
	pipelineInfo.extent = swapChain.GetExtent();

	renderer.GetPipelineHandler().Create(pipelineInfo, _pipeline, _pipelineLayout);

	VkDescriptorType uboTypes[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
	const uint32_t blockSize = 32 * SWAPCHAIN_MAX_FRAMES;
	uint32_t sizes[] = { blockSize, blockSize };
	_descriptorPool.Construct(_renderer, _layout, uboTypes, sizes, 2, blockSize);

	auto& meshHandler = renderer.GetMeshHandler();
	_mesh = meshHandler.Create(MeshHandler::GenerateQuad());
}

MaterialSystem::~MaterialSystem()
{
	_renderer.GetPipelineHandler().Destroy(_pipeline, _pipelineLayout);
	_renderer.GetLayoutHandler().DestroyLayout(_layout);
	_renderer.GetShaderExt().DestroyShader(_shader);
	_descriptorPool.Cleanup();
	_renderer.GetMeshHandler().Destroy(_mesh);
}

VkDescriptorSetLayout MaterialSystem::GetLayout() const
{
	return _layout;
}

void MaterialSystem::Update()
{
	auto& shaderHandler = _renderer.GetShaderHandler();
	auto& meshHandler = _renderer.GetMeshHandler();

	meshHandler.Bind(_mesh);

	for (const auto& [index, material] : *this)
	{
		const auto& transform = _transforms[index];
		shaderHandler.UpdatePushConstant(_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, transform);
		meshHandler.Draw();
	}
}
