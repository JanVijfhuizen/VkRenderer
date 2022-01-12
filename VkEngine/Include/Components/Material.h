﻿#pragma once
#include "Rendering/ShaderExt.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/MeshHandler.h"
#include "Rendering/SwapChainExt.h"
#include "Camera.h"

class TransformSystem;
class Renderer;

struct Material
{
	
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
	Shader _shader;
	DescriptorPool _descriptorPool{};
	Mesh _mesh;
	Texture _fallbackTexture;

	void DestroySwapChainAssets() const;
};
