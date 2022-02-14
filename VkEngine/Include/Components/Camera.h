#pragma once
#include "VkRenderer/VkHandlers/VkLayoutHandler.h"
#include "Rendering/UboAllocator.h"

class TransformSystem;
class VulkanRenderer;

struct Camera final
{
	struct alignas(256) Ubo final
	{
		glm::mat4 view;
		glm::mat4 projection;
	};

	glm::vec3 lookAt{ 0 };
	float fieldOfView = 45;
	float clipNear = .1f;
	float clipFar = 100;
};

/// <summary>
/// System that handles the camera components.
/// </summary>
class CameraSystem final : public ce::SmallSystem<Camera>
{
public:
	explicit CameraSystem(ce::Cecsar& cecsar, VulkanRenderer& renderer, TransformSystem& transforms, uint32_t capacity = 1);
	~CameraSystem();

	void Update();

	[[nodiscard]] VkDescriptorSet GetDescriptor(uint32_t sparseIndex) const;
	[[nodiscard]] VkDescriptorSetLayout GetLayout() const;
	[[nodiscard]] static vi::VkLayoutHandler::CreateInfo::Binding GetBindingInfo();

private:
	VulkanRenderer& _renderer;
	TransformSystem& _transforms;

	// Camera external layout, used in other rendering systems like the default material system.
	VkDescriptorSetLayout _layout;
	// Descriptor pool used for the cameras.
	VkDescriptorPool _descriptorPool;
	// Descriptor sets used per-frame (all the cameras are batched in one go).
	vi::ArrayPtr<VkDescriptorSet> _descriptorSets;
	// Allocator from which to get update bound ubos.
	UboAllocator<Camera::Ubo> _uboAllocator;
	// Ubos that are attached to the camera descriptor sets.
	vi::ArrayPtr<Camera::Ubo> _ubos;

	// Get the current descriptor index based on the swap chain image index.
	[[nodiscard]] uint32_t GetDescriptorStartIndex() const;
};
