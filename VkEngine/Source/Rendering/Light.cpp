#include "pch.h"
#include "Rendering/Light.h"
#include "Rendering/RenderManager.h"
#include "VkRenderer/VkRenderer.h"
#include "FileReader.h"
#include "Rendering/Vertex.h"
#include "Transform.h"
#include "Rendering/DescriptorPool.h"

Light::System::System(const Info& info) : MapSet<Light>(8), _info(info)
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();

	const auto vertCode = FileReader::Read("Shaders/shadowMapper.spv");
	_vertModule = renderer.CreateShaderModule(vertCode);

	vi::VkRenderer::LayoutInfo layoutInfo{};
	vi::VkRenderer::LayoutInfo::Binding lsm{};
	lsm.size = sizeof(Ubo);
	lsm.flag = VK_SHADER_STAGE_VERTEX_BIT;
	layoutInfo.bindings.push_back(lsm);
	_layout = renderer.CreateLayout(layoutInfo);

	vi::VkRenderer::RenderPassInfo renderPassInfo{};
	renderPassInfo.useColorAttachment = false;
	renderPassInfo.useDepthAttachment = true;
	renderPassInfo.depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	_renderPass = renderer.CreateRenderPass(renderPassInfo);

	vi::VkRenderer::PipelineInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.push_back(_layout);

	pipelineInfo.modules.push_back(
		{
			_vertModule,
			VK_SHADER_STAGE_VERTEX_BIT
		});

	pipelineInfo.pushConstants.push_back(
		{
			sizeof Transform::Baked,
			VK_SHADER_STAGE_VERTEX_BIT
		});

	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.extent =
	{
		static_cast<uint32_t>(info.shadowResolution.x),
		static_cast<uint32_t>(info.shadowResolution.y)
	};

	renderer.CreatePipeline(pipelineInfo, _pipeline, _pipelineLayout);
	_commandBuffer = renderer.CreateCommandBuffer();
	_fence = renderer.CreateFence();

	VkDescriptorType uboType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uint32_t size = 8;
	_descriptorPool = DescriptorPool(_layout, &uboType, &size, 1, 8);
}

Light::System::~System()
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();

	renderer.DestroyCommandBuffer(_commandBuffer);
	renderer.DestroyFence(_fence);
	renderer.DestroyLayout(_layout);
	renderer.DestroyPipeline(_pipeline, _pipelineLayout);
	renderer.DestroyRenderPass(_renderPass);
	renderer.DestroyShaderModule(_vertModule);
}
