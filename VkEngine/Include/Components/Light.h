#pragma once
#include "Rendering/SwapChainExt.h"
#include "Rendering/ShaderExt.h"
#include "Rendering/MeshHandler.h"
#include "Rendering/DescriptorPool.h"

class CameraSystem;

struct Light final
{
	
};

struct ShadowCaster final
{
	
};

class ShadowCasterSystem final : public ce::System<ShadowCaster>
{
public:
	explicit ShadowCasterSystem(ce::Cecsar& cecsar);
};

class LightSystem final : public ce::SmallSystem<Light>, SwapChainExt::Dependency
{
public:
	LightSystem(ce::Cecsar& cecsar, Renderer& renderer, CameraSystem& cameras, ShadowCasterSystem& shadowCasters, size_t size = 8);
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

	CameraSystem& _cameras;
	ShadowCasterSystem& _shadowCasters;

	VkDescriptorSetLayout _layout;
	Shader _shader;
	Mesh _mesh;
	DescriptorPool _descriptorPool{};

	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout;

	void CreateMesh();
	void OnRecreateSwapChainAssets() override;
	void DestroySwapChainAssets() const;
};
