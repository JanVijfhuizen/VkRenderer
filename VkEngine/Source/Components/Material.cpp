#include "pch.h"
#include "Components/Material.h"
#include "Rendering/Renderer.h"
#include "Components/Camera.h"
#include "Rendering/Vertex.h"
#include "Components/Transform.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/MeshHandler.h"

MaterialSystem::MaterialSystem(ce::Cecsar& cecsar, 
	Renderer& renderer, TransformSystem& transforms, 
	CameraSystem& cameras, const char* shaderName) : 
	System<Material>(cecsar), Dependency(renderer), 
	_renderer(renderer), _transforms(transforms), _cameras(cameras)
{
	_shader = renderer.GetShaderExt().Load(shaderName);

	vi::VkLayoutHandler::Info layoutInfo{};
	layoutInfo.bindings.Add(CameraSystem::GetBindingInfo());
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	VkDescriptorType uboTypes[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
	const uint32_t blockSize = 32 * SWAPCHAIN_MAX_FRAMES;
	uint32_t sizes[] = { blockSize, blockSize };
	_descriptorPool.Construct(_renderer, _layout, uboTypes, sizes, 2, blockSize);

	auto& meshHandler = renderer.GetMeshHandler();
	_mesh = meshHandler.Create(MeshHandler::GenerateQuad());

	OnRecreateSwapChainAssets();
}

MaterialSystem::~MaterialSystem()
{
	DestroySwapChainAssets();
	_renderer.GetLayoutHandler().DestroyLayout(_layout);
	_renderer.GetShaderExt().DestroyShader(_shader);
	_renderer.GetMeshHandler().Destroy(_mesh);
	_descriptorPool.Cleanup();
}

void MaterialSystem::OnRecreateSwapChainAssets()
{
	if (_pipeline)
		DestroySwapChainAssets();

	auto& swapChain = _renderer.GetSwapChain();

	vi::VkPipelineHandler::Info pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.Add(_layout);
	pipelineInfo.modules.Add(_shader.vertex);
	pipelineInfo.modules.Add(_shader.fragment);
	pipelineInfo.pushConstants.Add({ sizeof(Transform), VK_SHADER_STAGE_VERTEX_BIT });
	pipelineInfo.renderPass = swapChain.GetRenderPass();
	pipelineInfo.extent = swapChain.GetExtent();

	_renderer.GetPipelineHandler().Create(pipelineInfo, _pipeline, _pipelineLayout);
}

void MaterialSystem::DestroySwapChainAssets() const
{
	_renderer.GetPipelineHandler().Destroy(_pipeline, _pipelineLayout);
}

void MaterialSystem::Update()
{
	auto& descriptorPoolHandler = _renderer.GetDescriptorPoolHandler();
	auto& shaderHandler = _renderer.GetShaderHandler();
	auto& meshHandler = _renderer.GetMeshHandler();
	auto& pipelineHandler = _renderer.GetPipelineHandler();

	pipelineHandler.Bind(_pipeline, _pipelineLayout);
	meshHandler.Bind(_mesh);

	for (auto& [camIndex, camera] : _cameras)
	{
		auto set = _cameras.GetDescriptor(camera);
		descriptorPoolHandler.BindSets(&set, 1);

		for (const auto& [matIndex, material] : *this)
		{
			const auto& transform = _transforms[matIndex];
			shaderHandler.UpdatePushConstant(_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, transform);
			meshHandler.Draw();
		}
	}
}
