#pragma once
#include "VkRenderer/VkHandlers/VkLayoutHandler.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/UboPool.h"

class Renderer;

struct Camera final
{
	friend class CameraSystem;

	struct Ubo final
	{
		glm::vec3 position;
		float scale;
		float depth = 1;
	};

private:
	VkBuffer _buffer;
	VkDescriptorSet _descriptors[SWAPCHAIN_MAX_FRAMES];
};

constexpr auto MAX_CAMERAS = 8;

class CameraSystem final : public ce::SmallSystem<Camera>
{
public:
	explicit CameraSystem(ce::Cecsar& cecsar, Renderer& renderer);
	~CameraSystem();

	void Update();

	Camera& Insert(uint32_t sparseIndex, const Camera& value = {}) override;
	void RemoveAt(uint32_t index) override;

	[[nodiscard]] VkDescriptorSet GetDescriptor(const Camera& value) const;
	[[nodiscard]] static vi::VkLayoutHandler::Info::Binding GetBindingInfo();

private:
	Renderer& _renderer;
	VkDescriptorSetLayout _layout;
	DescriptorPool _descriptorPool{};
	UboPool<Camera::Ubo> _uboPool;
};
