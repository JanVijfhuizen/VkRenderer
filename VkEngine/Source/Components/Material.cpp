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
	auto& materialBinding = layoutInfo.bindings.Add();
	materialBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	materialBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	VkDescriptorType uboType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uint32_t blockSize = 32 * SWAPCHAIN_MAX_FRAMES;
	_descriptorPool.Construct(_renderer, _layout, &uboType, &blockSize, 1, blockSize);

	_mesh = renderer.GetMeshHandler().Create(MeshHandler::GenerateCube());
	_fallbackTexture = renderer.GetTextureHandler().Create("test", "png");

	OnRecreateSwapChainAssets();
}

MaterialSystem::~MaterialSystem()
{
	DestroySwapChainAssets();
	_renderer.GetLayoutHandler().DestroyLayout(_layout);
	_renderer.GetShaderExt().DestroyShader(_shader);
	_renderer.GetTextureHandler().Destroy(_fallbackTexture);
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
	pipelineInfo.setLayouts.Add(_cameras.GetLayout());
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
	auto& swapChain = _renderer.GetSwapChain();
	auto& swapChainext = _renderer.GetSwapChainExt();

	const uint32_t imageIndex = swapChain.GetImageIndex();

	pipelineHandler.Bind(_pipeline, _pipelineLayout);
	meshHandler.Bind(_mesh);

	union
	{
		struct
		{
			VkDescriptorSet camera;
			VkDescriptorSet material;
		};
		VkDescriptorSet values[2];
	} sets{};

	for (auto& [camIndex, camera] : _cameras)
	{
		sets.camera = _cameras.GetDescriptor(camera);

		for (const auto& [matIndex, material] : *this)
		{
			sets.material = material._descriptors[imageIndex];

			const auto sampler = shaderHandler.CreateSampler();
			const auto texture = material.texture ? material.texture : &_fallbackTexture;
			shaderHandler.BindSampler(sets.material, texture->imageView, texture->layout, sampler, 0, 0);
			
			descriptorPoolHandler.BindSets(sets.values, sizeof sets / sizeof(VkDescriptorSet));

			const auto& transform = _transforms[matIndex];
			shaderHandler.UpdatePushConstant(_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, transform);
			meshHandler.Draw();

			swapChainext.Collect(sampler);
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

Material& MaterialSystem::Insert(const uint32_t sparseIndex, const Material& value)
{
	auto& camera = System<Material>::Insert(sparseIndex, value);
	for (auto& _descriptor : camera._descriptors)
		_descriptor = _descriptorPool.Get();
	return camera;
}

void MaterialSystem::RemoveAt(const uint32_t index)
{
	auto& swapChainExt = _renderer.GetSwapChainExt();
	auto& camera = operator[](index);
	for (auto& descriptor : camera._descriptors)
		swapChainExt.Collect(descriptor, _descriptorPool);
	System<Material>::RemoveAt(index);
}
