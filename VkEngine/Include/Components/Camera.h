﻿#pragma once
#include "VkRenderer/VkHandlers/VkLayoutHandler.h"
#include "Rendering/UboPool.h"

class TransformSystem;
class Renderer;

struct Camera final
{
	struct alignas(256) Ubo final
	{
		glm::vec3 position;
		float rotation;
		float clipFar;
		float aspectRatio;
	};

	float clipFar = 100;
};

class CameraSystem final : public ce::SmallSystem<Camera>
{
public:
	explicit CameraSystem(ce::Cecsar& cecsar, Renderer& renderer, TransformSystem& transforms, uint32_t capacity = 8);
	~CameraSystem();

	void Update();

	[[nodiscard]] VkDescriptorSet GetDescriptor(uint32_t sparseIndex) const;
	[[nodiscard]] VkDescriptorSetLayout GetLayout() const;
	[[nodiscard]] static vi::VkLayoutHandler::CreateInfo::Binding GetBindingInfo();

private:
	Renderer& _renderer;
	TransformSystem& _transforms;

	VkDescriptorSetLayout _layout;
	VkDescriptorPool _descriptorPool;
	vi::ArrayPtr<VkDescriptorSet> _descriptorSets;
	UboPool<Camera::Ubo> _uboPool;
	vi::ArrayPtr<Camera::Ubo> _ubos;

	[[nodiscard]] uint32_t GetDescriptorStartIndex() const;
};
