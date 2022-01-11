#pragma once
#include "VkRenderer/VkHandlers/VkLayoutHandler.h"
#include "Rendering/DescriptorPool.h"

class Renderer;

struct Camera final
{
	struct Ubo final
	{
		glm::vec3 position;
		float scale;
		float depth = 1;
	};
};

constexpr auto MAX_CAMERAS = 8;

class CameraSystem final : public ce::SmallSystem<Camera>
{
public:
	explicit CameraSystem(ce::Cecsar& cecsar, Renderer& renderer);
	~CameraSystem();

	[[nodiscard]] static vi::VkLayoutHandler::Info::Binding GetBindingInfo();

private:
	Renderer& _renderer;
	VkDescriptorSetLayout _layout;
	DescriptorPool _descriptorPool{};
};
