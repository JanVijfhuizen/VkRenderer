#include "pch.h"
#include "Components/Material.h"
#include "Rendering/Renderer.h"
#include "Components/Camera.h"
#include "Rendering/Vertex.h"
#include "Components/Transform.h"

Material::System::System(ce::Cecsar& cecsar, Renderer& renderer, const char* shaderName) :
	ce::System<Material>(cecsar), _renderer(renderer)
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
}

Material::System::~System()
{
	_renderer.GetPipelineHandler().Destroy(_pipeline, _pipelineLayout);
	_renderer.GetLayoutHandler().DestroyLayout(_layout);
	_renderer.GetShaderExt().DestroyShader(_shader);
}

VkDescriptorSetLayout Material::System::GetLayout() const
{
	return _layout;
}
