#pragma once
#include "Rendering/ShaderExt.h"
#include "Rendering/MeshHandler.h"
#include "Rendering/SwapChainExt.h"
#include "Camera.h"

class TransformSystem;
class Renderer;

struct Material
{
	Texture* texture = nullptr;
};

class MaterialSystem final : public ce::System<Material>, SwapChainExt::Dependency
{
public:
	explicit MaterialSystem(ce::Cecsar& cecsar, Renderer& renderer, 
		TransformSystem& transforms, CameraSystem& cameras, const char* shaderName);
	~MaterialSystem();

	void Update();

	[[nodiscard]] Shader GetShader() const;
	[[nodiscard]] Mesh GetMesh() const;
	[[nodiscard]] Texture GetFallbackTexture() const;

protected:
	void OnRecreateSwapChainAssets() override;

private:
	Renderer& _renderer;
	TransformSystem& _transforms;
	CameraSystem& _cameras;

	VkDescriptorSetLayout _layout;
	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout;
	VkDescriptorPool _descriptorPool;
	vi::ArrayPtr<VkDescriptorSet> _descriptorSets;
	Shader _shader;
	Mesh _mesh;
	Texture _fallbackTexture;

	void DestroySwapChainAssets() const;
	[[nodiscard]] uint32_t GetDescriptorStartIndex() const;
};
