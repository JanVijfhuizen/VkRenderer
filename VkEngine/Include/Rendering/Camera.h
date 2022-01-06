﻿#pragma once
#include "MapSet.h"
#include "DescriptorPool.h"

struct Camera final
{
	glm::vec3 lookat{};
	float fieldOfView = 45;
	float clipNear = 0.1f;
	float clipFar = 1e3f;

	struct alignas(256) Ubo final
	{
		glm::mat4 view;
		glm::mat4 projection;
	};

	enum class Type
	{
		Orthographic,
		Perspective
	};

	class System final : public MapSet<Camera>, public Singleton<System>
	{
	public:
		System();
		~System();

		void Update();

		KeyValuePair<unsigned, Camera>& Add(const KeyValuePair<unsigned, Camera>& value) override;
		void EraseAt(size_t index) override;

		[[nodiscard]] Camera& GetMainCamera();
		[[nodiscard]] VkDescriptorSetLayout GetLayout() const;

	private:
		DescriptorPool _descriptorPool;
		VkDescriptorSetLayout _layout;
	};

	[[nodiscard]] VkDescriptorSet* GetDescriptors();

private:
	friend System;

	VkBuffer _buffer;
	VkDeviceMemory _memory;
	VkDescriptorSet _descriptors[SWAPCHAIN_MAX_FRAMES];
};