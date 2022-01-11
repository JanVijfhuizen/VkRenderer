#pragma once
#include "Rendering/ShaderExt.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/MeshHandler.h"

class TransformSystem;
class Renderer;

struct Material
{
	
};

class MaterialSystem final : public ce::System<Material>
{
public:
	explicit MaterialSystem(ce::Cecsar& cecsar, Renderer& renderer, 
		TransformSystem& transforms, const char* shaderName);
	~MaterialSystem();

	void RecreateVulkanDependencies();

	void Update();
	[[nodiscard]] VkDescriptorSetLayout GetLayout() const;

private:
	Renderer& _renderer;
	TransformSystem& _transforms;
	VkDescriptorSetLayout _layout;
	VkPipeline _pipeline;
	VkPipelineLayout _pipelineLayout;
	ShaderExt::Shader _shader;
	DescriptorPool _descriptorPool;
	MeshHandler::Mesh _mesh;
};
