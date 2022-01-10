#pragma once
#include "Rendering/ShaderExt.h"
#include "Rendering/DescriptorPool.h"

class Renderer;

struct Material
{
public:
	class System final : public ce::System<Material>
	{
	public:
		explicit System(ce::Cecsar& cecsar, Renderer& renderer, const char* shaderName);
		~System();

		[[nodiscard]] VkDescriptorSetLayout GetLayout() const;

	private:
		Renderer& _renderer;
		VkDescriptorSetLayout _layout;
		VkPipeline _pipeline;
		VkPipelineLayout _pipelineLayout;
		ShaderExt::Shader _shader;
		DescriptorPool _descriptorPool;
	};
};
