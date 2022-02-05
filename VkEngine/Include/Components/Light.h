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
	struct CubeMap final
	{
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	};

	CameraSystem& _cameras;
	MaterialSystem& _materials;
	ShadowCasterSystem& _shadowCasters;
	TransformSystem& _transforms;

	VkDescriptorSetLayout _layout;
	Shader _shader;
	DescriptorPool _descriptorPool{};
	vi::ArrayPtr<CubeMap> _cubeMaps{};

	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout;

	void CreateCubeMaps(vi::VkCoreSwapchain& swapChain, glm::ivec2 resolution);
	void DestroyCubeMaps();

	void OnRecreateSwapChainAssets() override;
	void DestroySwapChainAssets() const;
};
