#pragma once
#include "Rendering/SwapChainExt.h"
#include "Rendering/ShaderExt.h"
#include "Rendering/MeshHandler.h"
#include "Rendering/DescriptorPool.h"

class TransformSystem;
class MaterialSystem;
class CameraSystem;

struct Light final
{
	float range = 5;
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
	LightSystem(ce::Cecsar& cecsar, Renderer& renderer,
		CameraSystem& cameras, MaterialSystem& materials, 
		ShadowCasterSystem& shadowCasters, TransformSystem& transforms, size_t size = 8);
	~LightSystem();

	void Draw();

private:
	struct ShadowVertex final
	{
		uint32_t index;

		[[nodiscard]] static VkVertexInputBindingDescription GetBindingDescription();
		[[nodiscard]] static vi::Vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	};

	struct Ubo final
	{
		glm::vec2 vertices[4];
		glm::vec2 textureCoordinates[4];
		float height;
	};

	struct ShadowMap final
	{
		VkImage image;
		VkImageView view;
	};

	CameraSystem& _cameras;
	MaterialSystem& _materials;
	ShadowCasterSystem& _shadowCasters;
	TransformSystem& _transforms;

	VkDescriptorSetLayout _layout;
	Shader _shader;
	Mesh _mesh;
	DescriptorPool _descriptorPool{};
	vi::ArrayPtr<ShadowMap> _shadowMaps{};

	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout;

	void CreateMesh();
	void OnRecreateSwapChainAssets() override;
	void DestroySwapChainAssets() const;
};
