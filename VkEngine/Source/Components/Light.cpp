#include "pch.h"
#include "Components/Light.h"
#include "VkRenderer/VkHandlers/VkLayoutHandler.h"
#include "Rendering/Renderer.h"

LightSystem::LightSystem(ce::Cecsar& cecsar, Renderer& renderer, const size_t size) : 
	SmallSystem<Light>(cecsar, size), Dependency(renderer)
{
	_shader = renderer.GetShaderExt().Load("light-");

	vi::VkLayoutHandler::CreateInfo layoutInfo{};
	auto& materialBinding = layoutInfo.bindings.Add();
	materialBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	materialBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);
}

LightSystem::~LightSystem()
{
	renderer.GetShaderExt().DestroyShader(_shader);
	renderer.GetLayoutHandler().DestroyLayout(_layout);
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

void LightSystem::OnRecreateSwapChainAssets()
{

}

ShadowCasterSystem::ShadowCasterSystem(ce::Cecsar& cecsar) : System<ShadowCaster>(cecsar)
{
}
