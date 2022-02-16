#pragma once
#include "Rendering/SwapChainExt.h"
#include "Rendering/ShaderExt.h"
#include "Rendering/UboAllocator.h"

class TransformSystem;
class MaterialSystem;

/// <summary>
/// Component that can be used as a light source for other entities.
/// </summary>
struct Light final
{
	float range = 50;
};

/// <summary>
/// Component that can be used as a shadow caster for the lighting system.
/// </summary>
struct ShadowCaster final
{
	
};

/// <summary>
/// System that handles the shadow caster components.
/// </summary>
class ShadowCasterSystem final : public ce::System<ShadowCaster>
{
public:
	explicit ShadowCasterSystem(ce::Cecsar& cecsar);
};

/// <summary>
/// System that handles the light components.
/// </summary>
class LightSystem final : public ce::SmallSystem<Light>, SwapChainExt::Dependency
{
public:
	/// <summary>
	/// Struct used to construct the light system.
	/// </summary>
	struct Info final
	{
		// Maximum amount of lights present.
		// Make sure this corresponds to the shader code.
		size_t size = 6;
		// Individual face sizes of the cubemap the light is rendered to.
		glm::ivec2 shadowResolution{ 512 };
	};

	LightSystem(ce::Cecsar& cecsar, VulkanRenderer& renderer, MaterialSystem& materials,
		ShadowCasterSystem& shadowCasters, TransformSystem& transforms, const Info& info = {});
	~LightSystem();

	void Render(VkSemaphore waitSemaphore);

	/// <returns>Semaphore which triggers once all the lights have been drawn.</returns>
	[[nodiscard]] VkSemaphore GetRenderFinishedSemaphore() const;

	[[nodiscard]] VkDescriptorSetLayout GetLayout() const;
	[[nodiscard]] VkDescriptorSet GetDescriptorSet(uint32_t index) const;

private:
	// Contains buffers and memory to a depth-only cubemap.
	// This is used for point lights to render the scene from every direction (camera origin being the light itself).
	struct CubeMapDepthBuffer final
	{
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
		VkFramebuffer frameBuffer;
	};

	// Contains sync objects used to syncronize rendering with the next commands.
	struct Frame final
	{
		VkCommandBuffer commandBuffer;
		VkSemaphore signalSemaphore;
	};

	// UBO that contains a view matrix for every cubemap face, all multiplied by a projection matrix.
	struct alignas(512) GeometryUbo final
	{
		glm::mat4 matrices[6]{};
	};

	// UBO that contains data for a single light.
	struct FragmentLightUbo final
	{
		glm::vec3 position;
		float range;
	};

	// UBO that contains data for lighting purposes.
	struct FragmentLightingUbo final
	{
		uint32_t count;
	};

	struct PushConstant final
	{
		glm::mat4 modelMatrix;
		uint32_t index;
	};

	MaterialSystem& _materials;
	ShadowCasterSystem& _shadowCasters;
	TransformSystem& _transforms;

	// Resolution per-face for the cubemap.
	glm::ivec2 _shadowResolution;
	Shader _shader;
	// Descriptor set per-light for the cubemap rendering.
	vi::ArrayPtr<VkDescriptorSet> _descriptorSets;
	VkDescriptorPool _descriptorPool;

	// External descriptor set that can be used to attach the rendered cubemaps to another renderer.
	vi::ArrayPtr<VkDescriptorSet> _extDescriptorSets;
	VkDescriptorPool _extDescriptorPool;
	// External samplers for the cubemaps.
	vi::ArrayPtr<VkSampler> _extSamplers;

	UboAllocator<GeometryUbo> _geometryUboAllocator;
	UboAllocator<FragmentLightUbo> _fragmentLightUboAllocator;
	UboAllocator<FragmentLightingUbo> _fragmentLightingUboAllocator;
	vi::ArrayPtr<GeometryUbo> _geometryUbos;
	vi::ArrayPtr<FragmentLightUbo> _fragmentUbos;

	// Per-frame syncronization objects.
	vi::ArrayPtr<Frame> _frames;
	// Depth-only cubemaps that can be used for shadow mapping.
	vi::ArrayPtr<CubeMapDepthBuffer> _cubeMaps;

	// Internal layout.
	VkDescriptorSetLayout _layout;
	// External layout.
	VkDescriptorSetLayout _extLayout;

	VkRenderPass _renderPass;
	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout;

	void CreateCubeMaps(vi::VkCoreSwapchain& swapChain, glm::ivec2 resolution);
	void DestroyCubeMaps();

	// Sets up the buffers to make sure the cubemaps can be used externally.
	void CreateExtDescriptorDependencies();
	void DestroyExtDescriptorDependencies() const;

	void OnRecreateSwapChainAssets() override;
	void DestroySwapChainAssets() const;
};
