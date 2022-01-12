#pragma once
#include "VkRenderer/VkHandlers/VkLayoutHandler.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/UboPool.h"

class TransformSystem;
class Renderer;

struct Camera final
{
	friend class CameraSystem;

	struct alignas(256) Ubo final
	{
		glm::vec3 position;
		float rotation;
		float clipFar;
	};

	float clipFar = 100;

private:
	VkDescriptorSet _descriptors[SWAPCHAIN_MAX_FRAMES];
};

constexpr auto MAX_CAMERAS = 8;

class CameraSystem final : public ce::SmallSystem<Camera>
{
public:
	explicit CameraSystem(ce::Cecsar& cecsar, Renderer& renderer, TransformSystem& transforms);
	~CameraSystem();

	void Update();

	Camera& Insert(uint32_t sparseIndex, const Camera& value = {}) override;
	void RemoveAt(uint32_t index) override;

	[[nodiscard]] VkDescriptorSet GetDescriptor(const Camera& value) const;
	[[nodiscard]] static vi::VkLayoutHandler::Info::Binding GetBindingInfo();

private:
	Renderer& _renderer;
	TransformSystem& _transforms;

	VkDescriptorSetLayout _layout;
	DescriptorPool _descriptorPool{};
	UboPool<Camera::Ubo> _uboPool;
	vi::ArrayPtr<Camera::Ubo> _ubos;
};
