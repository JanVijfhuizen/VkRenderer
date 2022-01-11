#pragma once
#include "Rendering/ShaderExt.h"
#include "Rendering/DescriptorPool.h"

class Renderer;

struct Material
{
	
};

class MaterialSystem final : public ce::System<Material>
{
public:
	explicit MaterialSystem(ce::Cecsar& cecsar, Renderer& renderer, const char* shaderName);
	~MaterialSystem();

	void Update();
	[[nodiscard]] VkDescriptorSetLayout GetLayout() const;

private:
	Renderer& _renderer;
	VkDescriptorSetLayout _layout;
	VkPipeline _pipeline;
	VkPipelineLayout _pipelineLayout;
	ShaderExt::Shader _shader;
	DescriptorPool _descriptorPool;
};
