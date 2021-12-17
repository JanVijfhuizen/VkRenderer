#pragma once
#include "MapSet.h"

struct Camera final
{
	struct Ubo final
	{
		glm::mat4 view;
		glm::mat4 projection;
	};

	enum class Type
	{
		Orthographic,
		Perspective
	};

	class System final : MapSet<Camera>, public Singleton<System>
	{
	public:
		System();
		void Update();

		KeyValuePair<unsigned, Camera>& Add(const KeyValuePair<unsigned, Camera>& value) override;
		void EraseAt(size_t index) override;
	};

	glm::vec3 lookat{};
	float fieldOfView = 45;
	float clipNear = 0.1f;
	float clipFar = 1e3f;

private:
	friend System;

	VkBuffer _buffer;
	VkDeviceMemory _memory;
	VkDescriptorSet _descriptor;
};
