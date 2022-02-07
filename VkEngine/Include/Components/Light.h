#pragma once
#include "Rendering/SwapChainExt.h"
#include "Rendering/MeshHandler.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/ShaderExt.h"

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
	struct Info final
	{
		size_t size = 8;
		glm::ivec2 shadowResolution{ 512 };
	};

	LightSystem(ce::Cecsar& cecsar, Renderer& renderer,
		CameraSystem& cameras, MaterialSystem& materials, 
		ShadowCasterSystem& shadowCasters, TransformSystem& transforms, const Info& info = {});
	~LightSystem();

	void Draw();

private:
	struct DepthBuffer final
	{
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	};

	struct Ubo final
	{
		glm::mat4 matrices[6]{};
	};

	CameraSystem& _cameras;
	MaterialSystem& _materials;
	ShadowCasterSystem& _shadowCasters;
	TransformSystem& _transforms;

	Shader _shader;
	vi::ArrayPtr<DepthBuffer> _cubeMaps{};
	vi::ArrayPtr<DepthBuffer> _renderTargets{};

	VkRenderPass _renderPass;
	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout;

	void CreateRenderTargets(vi::VkCoreSwapchain& swapChain, glm::ivec2 resolution);
	void DestroyRenderTargets();
	void CreateCubeMaps(vi::VkCoreSwapchain& swapChain, glm::ivec2 resolution);
	void DestroyCubeMaps();

	void OnRecreateSwapChainAssets() override;
	void DestroySwapChainAssets() const;
};
