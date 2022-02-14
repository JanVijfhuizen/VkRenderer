#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"
#include "SwapChainExt.h"
#include "ShaderExt.h"
#include "DescriptorPool.h"
#include "MeshHandler.h"

class PostEffectHandler;
class VulkanRenderer;

/// <summary>
/// Inherit from this to create a post effect from scratch.
/// </summary>
class PostEffect
{
	friend PostEffectHandler;

public:
	/// <summary>
	/// Struct that holds render assets to be used for the draw call.
	/// </summary>
	struct Frame final
	{
		VkImage colorImage;
		VkImage depthImage;
		VkDeviceMemory colorMemory;
		VkDeviceMemory depthMemory;
		// Render target.
		VkFramebuffer frameBuffer;
		// Reusable command buffer.
		VkCommandBuffer commandBuffer;
		// Triggers on render finished.
		VkSemaphore renderFinishedSemaphore;

		union
		{
			struct
			{
				// Color image view.
				VkImageView imageView;
				// Depth image view.
				VkImageView depthImageView;
			};
			VkImageView imageViews[2]
			{
				VK_NULL_HANDLE,
				VK_NULL_HANDLE
			};
		};

		// Reusable descriptor set.
		VkDescriptorSet descriptorSet;
	};

	// Post effect specific method to inherit from.
	virtual void Render(Frame& frame) = 0;

protected:
	VulkanRenderer& renderer;

	explicit PostEffect(VulkanRenderer& renderer);
	// Called by the swap chain recreation event and startup.
	virtual void OnRecreateAssets() = 0;
	// Called by the swap chain recreation event and cleanup.
	virtual void DestroyAssets() = 0;
};

/// <summary>
/// Inherit from this to create a standardized post effect with some stuff already set up.
/// </summary>
class BasicPostEffect final : public PostEffect
{
public:
	explicit BasicPostEffect(VulkanRenderer& renderer, const char* shaderName);
	~BasicPostEffect();

	void Render(Frame& frame) override;

private:
	Shader _shader;
	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout;

	void OnRecreateAssets() override;
	void DestroyAssets() override;
};

/// <summary>
/// Contains engine specific post effect methods.
/// </summary>
class PostEffectHandler final : public vi::VkHandler, public SwapChainExt::Dependency
{
public:
	explicit PostEffectHandler(VulkanRenderer& renderer, VkSampleCountFlagBits msaaSamples);
	~PostEffectHandler();

	void BeginFrame(VkSemaphore waitSemaphore);
	void EndFrame();

	void Render() const;

	/// <returns>Semaphore which triggers once all the post effects have been drawn.</returns>
	[[nodiscard]] VkSemaphore GetRenderFinishedSemaphore() const;
	[[nodiscard]] VkRenderPass GetRenderPass() const;
	[[nodiscard]] glm::ivec2 GetExtent() const;
	[[nodiscard]] VkDescriptorSetLayout GetLayout() const;
	/// <returns>Render quad.</returns>
	[[nodiscard]] Mesh& GetMesh();

	void Add(PostEffect* postEffect);

	[[nodiscard]] bool IsEmpty() const;

protected:
	void OnRecreateSwapChainAssets() override;

private:
	VulkanRenderer& _renderer;
	VkSampleCountFlagBits _msaaSamples;
	VkRenderPass _renderPass = VK_NULL_HANDLE;
	glm::ivec2 _extent;

	uint32_t _imageIndex;

	Shader _shader;
	VkDescriptorSetLayout _layout;
	Mesh _mesh;
	DescriptorPool _descriptorPool{};

	vi::Vector<PostEffect*> _postEffects{4, GMEM_VOL};
	// Frames for all the swapchain images. Sequenced linearly like so:
	// [Swap chain image 0: [0, 1, 2 ...], Swap chain image 1: [0, 1, ...]]
	vi::Vector<PostEffect::Frame> _frames;

	// Semaphore to wait for before the drawing starts.
	VkSemaphore _waitSemaphore;

	void LayerBeginFrame(uint32_t index);
	void LayerEndFrame(uint32_t index) const;

	void RecreateLayerAssets(uint32_t index);
	void DestroyLayerAssets(uint32_t index, bool calledByDestructor) const;
	void DestroySwapChainAssets(bool calledByDestructor) const;

	// Get the first layer frame based on the given swap chain image index.
	[[nodiscard]] PostEffect::Frame& GetStartFrame(uint32_t index) const;
	// Get the active layer frame based on the given swap chain image index.
	[[nodiscard]] PostEffect::Frame& GetActiveFrame(uint32_t index) const;
};
