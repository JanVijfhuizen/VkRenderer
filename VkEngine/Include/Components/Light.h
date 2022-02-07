#pragma once
#include "Rendering/SwapChainExt.h"
#include "Rendering/ShaderExt.h"
#include "Rendering/UboPool.h"

class TransformSystem;
class MaterialSystem;

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
	struct Info final
	{
		size_t size = 8;
		glm::ivec2 shadowResolution{ 512 };
	};

	LightSystem(ce::Cecsar& cecsar, Renderer& renderer, MaterialSystem& materials, 
		ShadowCasterSystem& shadowCasters, TransformSystem& transforms, const Info& info = {});
	~LightSystem();

	void Render(VkSemaphore waitSemaphore);

	[[nodiscard]] VkSemaphore GetRenderFinishedSemaphore();

private:
	struct DepthBuffer final
	{
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
		VkFramebuffer frameBuffer;
	};

	struct Frame final
	{
		DepthBuffer cubeMap;
		VkCommandBuffer commandBuffer;
		VkSemaphore signalSemaphore;
	};

	struct alignas(512) GeometryUbo final
	{
		glm::mat4 matrices[6]{};
	};

	struct alignas(256) FragmentUbo final
	{
		glm::vec3 position;
		float range;
	};

	MaterialSystem& _materials;
	ShadowCasterSystem& _shadowCasters;
	TransformSystem& _transforms;

	glm::ivec2 _shadowResolution;
	Shader _shader;
	vi::ArrayPtr<VkDescriptorSet> _descriptorSets;
	VkDescriptorPool _descriptorPool;

	UboPool<GeometryUbo> _geometryUboPool;
	UboPool<GeometryUbo> _fragmentUboPool;
	vi::ArrayPtr<GeometryUbo> _geometryUbos;
	vi::ArrayPtr<FragmentUbo> _fragmentUbos;
	vi::ArrayPtr<Frame> _frames;

	VkDescriptorSetLayout _layout;
	VkRenderPass _renderPass;
	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout;

	void CreateCubeMaps(vi::VkCoreSwapchain& swapChain, glm::ivec2 resolution);
	void DestroyCubeMaps();

	void OnRecreateSwapChainAssets() override;
	void DestroySwapChainAssets() const;
};
