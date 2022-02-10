﻿#pragma once
#include "Rendering/SwapChainExt.h"
#include "Rendering/ShaderExt.h"
#include "Rendering/UboAllocator.h"

class TransformSystem;
class MaterialSystem;

struct Light final
{
	float range = 50;
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
		size_t size = 6;
		glm::ivec2 shadowResolution{ 512 };
	};

	LightSystem(ce::Cecsar& cecsar, Renderer& renderer,
		ShadowCasterSystem& shadowCasters, TransformSystem& transforms, const Info& info = {});
	~LightSystem();

	void Render(VkSemaphore waitSemaphore, MaterialSystem& materials);

	[[nodiscard]] VkSemaphore GetRenderFinishedSemaphore() const;

	[[nodiscard]] VkDescriptorSetLayout GetLayout() const;
	[[nodiscard]] VkDescriptorSet GetDescriptorSet(uint32_t index) const;

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

	ShadowCasterSystem& _shadowCasters;
	TransformSystem& _transforms;

	glm::ivec2 _shadowResolution;
	Shader _shader;
	vi::ArrayPtr<VkDescriptorSet> _descriptorSets;
	VkDescriptorPool _descriptorPool;

	vi::ArrayPtr<VkDescriptorSet> _extDescriptorSets;
	VkDescriptorPool _extDescriptorPool;
	vi::ArrayPtr<VkSampler> _extSamplers;

	UboAllocator<GeometryUbo> _geometryUboPool;
	UboAllocator<GeometryUbo> _fragmentUboPool;
	vi::ArrayPtr<GeometryUbo> _geometryUbos;
	vi::ArrayPtr<FragmentUbo> _fragmentUbos;
	vi::ArrayPtr<Frame> _frames;
	vi::ArrayPtr<DepthBuffer> _cubeMaps;

	VkDescriptorSetLayout _layout;
	VkDescriptorSetLayout _extLayout;
	VkRenderPass _renderPass;
	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout;

	VkBuffer _currentFragBuffer;

	void CreateCubeMaps(vi::VkCoreSwapchain& swapChain, glm::ivec2 resolution);
	void DestroyCubeMaps();

	void CreateExtDescriptorDependencies();
	void DestroyExtDescriptorDependencies() const;

	void OnRecreateSwapChainAssets() override;
	void DestroySwapChainAssets() const;
};
