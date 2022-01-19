#pragma once
#include "Rendering/SwapChainExt.h"
#include "Rendering/ShaderExt.h"
#include "Rendering/MeshHandler.h"

struct Light final
{
	
};

struct ShadowCaster final
{
	
};

class LightSystem final : public ce::SmallSystem<Light>, SwapChainExt::Dependency
{
public:
	LightSystem(ce::Cecsar& cecsar, Renderer& renderer, size_t size = 8);
	~LightSystem();

private:
	struct ShadowVertex final
	{
		uint32_t index;
		glm::vec2 textureCoordinates;

		[[nodiscard]] static VkVertexInputBindingDescription GetBindingDescription();
		[[nodiscard]] static vi::Vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	};

	struct Ubo final
	{
		glm::vec3 vertices[4];
	};

	VkDescriptorSetLayout _layout;
	Shader _shader;
	Mesh _mesh;

	void CreateMesh();
	void OnRecreateSwapChainAssets() override;
};

class ShadowCasterSystem final : public ce::System<ShadowCaster>
{
public:
	explicit ShadowCasterSystem(ce::Cecsar& cecsar);
};
