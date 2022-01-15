#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"
#include "SwapChainExt.h"
#include "ShaderExt.h"
#include "DescriptorPool.h"
#include "MeshHandler.h"

class PostEffectHandler;
class Renderer;

class PostEffect
{
	friend PostEffectHandler;

public:
	struct Frame final
	{
		VkImage colorImage;
		VkImage depthImage;
		VkDeviceMemory colorMemory;
		VkDeviceMemory depthMemory;
		VkFramebuffer frameBuffer;
		VkCommandBuffer commandBuffer;
		VkFence fence;

		union
		{
			struct
			{
				VkImageView imageView;
				VkImageView depthImageView;
			};
			VkImageView imageViews[2]
			{
				VK_NULL_HANDLE,
				VK_NULL_HANDLE
			};
		};

		VkDescriptorSet descriptorSet;
	};

	virtual void Draw(Frame& frame) = 0;

protected:
	Renderer& renderer;

	explicit PostEffect(Renderer& renderer);
	virtual void OnRecreateAssets() = 0;
};

class MSAA final : public PostEffect
{
public:
	explicit MSAA(Renderer& renderer);
	~MSAA();

	void Draw(Frame& frame) override;

private:
	Shader _shader;
	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout;

	void OnRecreateAssets() override;
	void DestroyAssets();
};

class PostEffectHandler final : public vi::VkHandler, public SwapChainExt::Dependency
{
public:
	explicit PostEffectHandler(Renderer& renderer);
	~PostEffectHandler();

	void BeginFrame();
	void EndFrame();

	void Draw() const;

	[[nodiscard]] VkRenderPass GetRenderPass() const;
	[[nodiscard]] VkExtent2D GetExtent() const;
	[[nodiscard]] VkDescriptorSetLayout GetLayout() const;
	[[nodiscard]] Mesh& GetMesh();

	void Add(PostEffect* postEffect);

protected:
	void OnRecreateSwapChainAssets() override;

private:
	struct Layer final
	{
		PostEffect* postEffect = nullptr;
		PostEffect::Frame frames[SWAPCHAIN_MAX_FRAMES];
	};

	Renderer& _renderer;
	VkRenderPass _renderPass = VK_NULL_HANDLE;
	VkExtent2D _extent;

	uint32_t _imageIndex;

	Shader _shader;
	VkDescriptorSetLayout _layout;
	Mesh _mesh;
	DescriptorPool _descriptorPool{};

	vi::Vector<Layer> _layers{ 4, GMEM_VOL };

	void RecreateLayerAssets(Layer& layer);
	void DestroyLayerAssets(Layer& layer) const;
	void DestroySwapChainAssets() const;
};
