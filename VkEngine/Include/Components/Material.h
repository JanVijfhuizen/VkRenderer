#pragma once
#include "Rendering/ShaderExt.h"
#include "Rendering/MeshHandler.h"
#include "Rendering/SwapChainExt.h"
#include "Rendering/TextureHandler.h"

class CameraSystem;
class LightSystem;
class TransformSystem;
class VulkanRenderer;

/// <summary>
/// Component that handles the standard rendering for an entity.
/// </summary>
struct Material
{
	Mesh* mesh = nullptr;
	Texture* texture = nullptr;
};

/// <summary>
/// System that handles the material components.
/// </summary>
class MaterialSystem final : public ce::System<Material>, SwapChainExt::Dependency
{
public:
	explicit MaterialSystem(ce::Cecsar& cecsar, VulkanRenderer& renderer, 
		CameraSystem& cameras, LightSystem& lights, TransformSystem& transforms, const char* shaderName);
	~MaterialSystem();

	void Draw();

	[[nodiscard]] Shader& GetShader();
	[[nodiscard]] Mesh& GetFallbackMesh();
	[[nodiscard]] Texture& GetFallbackTexture();

protected:
	void OnRecreateSwapChainAssets() override;

private:
	CameraSystem& _cameras;
	LightSystem& _lights;
	TransformSystem& _transforms;

	VkDescriptorSetLayout _layout;
	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout;
	VkDescriptorPool _descriptorPool;
	vi::ArrayPtr<VkDescriptorSet> _descriptorSets;
	Shader _shader;
	Mesh _fallbackMesh;
	Texture _fallbackTexture;

	void DestroySwapChainAssets() const;
	[[nodiscard]] uint32_t GetDescriptorStartIndex() const;
};
