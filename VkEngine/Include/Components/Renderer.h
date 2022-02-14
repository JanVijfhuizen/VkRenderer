#pragma once
#include "Rendering/ShaderExt.h"
#include "Rendering/SwapChainExt.h"

class CameraSystem;
class LightSystem;
class MaterialSystem;
class TransformSystem;
class VulkanRenderer;

/// <summary>
/// Component that handles the standard rendering for an entity.
/// </summary>
struct Renderer final
{
	
};

/// <summary>
/// System that handles the render components.
/// </summary>
class RenderSystem final : public ce::System<Renderer>, SwapChainExt::Dependency
{
public:
	explicit RenderSystem(ce::Cecsar& cecsar, VulkanRenderer& renderer, MaterialSystem& materials,
		CameraSystem& cameras, LightSystem& lights, TransformSystem& transforms, const char* shaderName = "");
	~RenderSystem();

	void Draw();

	/// <returns>Shader used for this renderer.<returns>
	[[nodiscard]] Shader& GetShader();

protected:
	void OnRecreateSwapChainAssets() override;

private:
	CameraSystem& _cameras;
	LightSystem& _lights;
	MaterialSystem& _materials;
	TransformSystem& _transforms;

	VkDescriptorSetLayout _layout;
	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout;
	VkDescriptorPool _descriptorPool;
	vi::ArrayPtr<VkDescriptorSet> _descriptorSets;
	Shader _shader;

	void DestroySwapChainAssets() const;
	[[nodiscard]] uint32_t GetDescriptorStartIndex() const;
};
