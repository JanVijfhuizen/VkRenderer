#include "pch.h"
#include "Rendering/PostEffectHandler.h"
#include "VkRenderer/VkHandlers/VkRenderPassHandler.h"
#include "Rendering/VulkanRenderer.h"
#include "VkRenderer/VkCore/VkCorePhysicalDevice.h"
#include "VkRenderer/VkHandlers/VkCommandBufferHandler.h"
#include "VkRenderer/VkHandlers/VkSyncHandler.h"
#include "VkRenderer/VkCore/VkCoreSwapchain.h"
#include "VkRenderer/VkHandlers/VkImageHandler.h"
#include "VkRenderer/VkHandlers/VkMemoryHandler.h"
#include "VkRenderer/VkHandlers/VkFrameBufferHandler.h"
#include "VkRenderer/VkHandlers/VkLayoutHandler.h"
#include "VkRenderer/VkHandlers/VkShaderHandler.h"
#include "VkRenderer/VkHandlers/VkDescriptorPoolHandler.h"

PostEffect::PostEffect(VulkanRenderer& renderer) : renderer(renderer)
{

}

BasicPostEffect::BasicPostEffect(VulkanRenderer& renderer, const char* shaderName) : PostEffect(renderer)
{
	// Load shader based on the given name.
	auto& shaderExt = renderer.GetShaderExt();
	_shader = shaderExt.Load(shaderName);
}

BasicPostEffect::~BasicPostEffect()
{
	DestroyAssets();

	auto& shaderExt = renderer.GetShaderExt();
	shaderExt.DestroyShader(_shader);
}

void BasicPostEffect::Render(Frame& frame)
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& shaderHandler = renderer.GetShaderHandler();
	auto& meshHandler = renderer.GetMeshHandler();
	auto& pipelineHandler = renderer.GetPipelineHandler();
	auto& postEffectHandler = renderer.GetPostEffectHandler();
	auto& swapChainext = renderer.GetSwapChainExt();

	// Bind pipeline and render quad.
	pipelineHandler.Bind(_pipeline, _pipelineLayout);
	meshHandler.Bind(postEffectHandler.GetMesh());

	// Create a color sampler.
	const auto sampler = shaderHandler.CreateSampler();
	vi::VkShaderHandler::SamplerBindInfo colorBindInfo{};
	colorBindInfo.set = frame.descriptorSet;
	colorBindInfo.imageView = frame.imageView;
	colorBindInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	colorBindInfo.sampler = sampler;
	colorBindInfo.bindingIndex = 0;
	shaderHandler.BindSampler(colorBindInfo);

	// Create a depth sampler.
	vi::VkShaderHandler::SamplerCreateInfo depthSamplerCreateInfo{};
	depthSamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	depthSamplerCreateInfo.adressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	const auto depthSampler = shaderHandler.CreateSampler(depthSamplerCreateInfo);
	vi::VkShaderHandler::SamplerBindInfo depthBindInfo{};
	depthBindInfo.set = frame.descriptorSet;
	depthBindInfo.imageView = frame.depthImageView;
	depthBindInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	depthBindInfo.sampler = depthSampler;
	depthBindInfo.bindingIndex = 1;
	shaderHandler.BindSampler(depthBindInfo);

	// Bind descriptor sets and draw post effect quad.
	// Inputs are the color and depth images of the previous pass.
	descriptorPoolHandler.BindSets(&frame.descriptorSet, 1);
	meshHandler.Draw();

	swapChainext.Collect(sampler);
	swapChainext.Collect(depthSampler);
}

void BasicPostEffect::OnRecreateAssets()
{
	auto& swapChain = renderer.GetSwapChain();
	auto& pipelineHandler = renderer.GetPipelineHandler();
	auto& postEffectHandler = renderer.GetPostEffectHandler();

	// Create a pipeline based on the created shader in startup.
	vi::VkPipelineHandler::CreateInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.Add(postEffectHandler.GetLayout());
	for (auto& module : _shader.modules)
		pipelineInfo.modules.Add(module);
	// Use the standard swapchain renderpass and extent.
	pipelineInfo.renderPass = swapChain.GetRenderPass();
	pipelineInfo.extent = swapChain.GetExtent();

	pipelineHandler.Create(pipelineInfo, _pipeline, _pipelineLayout);
}

void BasicPostEffect::DestroyAssets()
{
	auto& pipelineHandler = renderer.GetPipelineHandler();
	pipelineHandler.Destroy(_pipeline, _pipelineLayout);
}

PostEffectHandler::PostEffectHandler(VulkanRenderer& renderer, const VkSampleCountFlagBits msaaSamples) : 
	VkHandler(renderer), Dependency(renderer), _renderer(renderer), _msaaSamples(msaaSamples)
{
	auto& meshHandler = renderer.GetMeshHandler();
	auto& swapChain = renderer.GetSwapChain();

	// Define layout with a color and depth texture input.
	vi::VkLayoutHandler::CreateInfo layoutInfo{};
	auto& samplerBinding = layoutInfo.bindings.Add();
	samplerBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutInfo.bindings.Add(samplerBinding);
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	VkDescriptorType uboType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uint32_t blockSize = 8 * swapChain.GetLength();
	_descriptorPool.Construct(_renderer, _layout, &uboType, &blockSize, 1, blockSize / 2);

	// Create render quad.
	_mesh = meshHandler.Create(MeshHandler::GenerateQuad());
	_frames.Reallocate(_postEffects.GetLength() * swapChain.GetLength(), GMEM_VOL);

	OnRecreateSwapChainAssets();
}

PostEffectHandler::~PostEffectHandler()
{
	auto& layoutHandler = _renderer.GetLayoutHandler();
	auto& meshHandler = _renderer.GetMeshHandler();

	layoutHandler.DestroyLayout(_layout);
	meshHandler.Destroy(_mesh);
	DestroySwapChainAssets(true);
	_descriptorPool.Cleanup();
}

void PostEffectHandler::BeginFrame(const VkSemaphore waitSemaphore)
{
	_waitSemaphore = waitSemaphore;
	// Render everything to the first render pass.
	LayerBeginFrame(0);
}

void PostEffectHandler::EndFrame()
{
	auto& swapChain = core.GetSwapChain();

	_imageIndex = swapChain.GetImageIndex();

	const uint32_t swapChainLength = swapChain.GetLength();
	const uint32_t count = _postEffects.GetCount();

	// Iterate and execute every post effect render pass.
	for (uint32_t i = 0; i < count; ++i)
	{
		auto& postEffect = _postEffects[i];
		auto& frame = _frames[swapChainLength * i + _imageIndex];

		// End current render pass.
		LayerEndFrame(i);
		if(i < count - 1)
		{
			// Begin new render pass.
			LayerBeginFrame(i + 1);
			postEffect->Render(frame);
		}
	}
}

void PostEffectHandler::Render() const
{
	// Draw the final render pass directly to the swap chain.
	const uint32_t index = _postEffects.GetCount() - 1;
	auto& finalLayer = _postEffects[index];
	finalLayer->Render(GetActiveFrame(index));
}

VkSemaphore PostEffectHandler::GetRenderFinishedSemaphore() const
{
	// Get the render finished semaphore from the last post effect layer.
	const uint32_t index = _postEffects.GetCount() - 1;
	return GetActiveFrame(index).renderFinishedSemaphore;
}

VkRenderPass PostEffectHandler::GetRenderPass() const
{
	return _renderPass;
}

glm::ivec2 PostEffectHandler::GetExtent() const
{
	return _extent;
}

VkDescriptorSetLayout PostEffectHandler::GetLayout() const
{
	return _layout;
}

Mesh& PostEffectHandler::GetMesh()
{
	return _mesh;
}

void PostEffectHandler::Add(PostEffect* postEffect)
{
	// Add a new post effect and create the layer assets for it.
	_postEffects.Add(postEffect);

	auto& swapChain = _renderer.GetSwapChain();
	const uint32_t swapChainLength = swapChain.GetLength();

	_frames.Resize(_frames.GetCount() + swapChainLength);
	RecreateLayerAssets(_postEffects.GetCount() - 1);
}

bool PostEffectHandler::IsEmpty() const
{
	return _postEffects.GetCount() == 0;
}

void PostEffectHandler::OnRecreateSwapChainAssets()
{
	// If the render pass has been created before, it means that the previous assets need to be destroyed.
	if (_renderPass)
		DestroySwapChainAssets(false);

	auto& renderPassHandler = _renderer.GetRenderPassHandler();
	auto& swapChain = _renderer.GetSwapChain();

	// This could basically use the same render pass as the swapchain, but I'm explicitely making it here too just to be safe.
	// In case I ever change the swapchain render pass this would still work.
	vi::VkRenderPassHandler::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.useColorAttachment = true;
	renderPassCreateInfo.colorFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	renderPassCreateInfo.useDepthAttachment = true;
	renderPassCreateInfo.depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	renderPassCreateInfo.depthFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	_renderPass = renderPassHandler.Create(renderPassCreateInfo);
	// Update post effect resolution.
	_extent = swapChain.GetExtent();

	for (uint32_t i = 0; i < _postEffects.GetCount(); ++i)
		RecreateLayerAssets(i);
}

void PostEffectHandler::LayerBeginFrame(const uint32_t index)
{
	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& renderPassHandler = core.GetRenderPassHandler();
	auto& swapChain = core.GetSwapChain();

	// Get the current swap chain image index.
	_imageIndex = swapChain.GetImageIndex();

	auto& frame = GetActiveFrame(index);
	commandBufferHandler.BeginRecording(frame.commandBuffer);

	VkClearValue clearColors[2];
	clearColors[0].color = { 0, 0, 0, 1 };
	clearColors[1].depthStencil = { 1, 0 };

	// Begin this frame's render pass.
	renderPassHandler.Begin(frame.frameBuffer, _renderPass, {},
		_extent, clearColors, 2);
}

void PostEffectHandler::LayerEndFrame(const uint32_t index) const
{
	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& renderPassHandler = core.GetRenderPassHandler();

	auto& frame = GetActiveFrame(index);

	renderPassHandler.End();
	commandBufferHandler.EndRecording();

	// Submit the swap chain layer render.
	// The goal here is to send all layer submits to the GPU without having to stall the CPU.
	// Therefore I'm using semaphores instead of fences to sequence them correctly.
	vi::VkCommandBufferHandler::SubmitInfo info{};
	info.buffers = &frame.commandBuffer;
	info.buffersCount = 1;
	info.waitSemaphore = index == 0 ? _waitSemaphore : GetActiveFrame(index -1).renderFinishedSemaphore;
	info.signalSemaphore = frame.renderFinishedSemaphore;
	info.fence = VK_NULL_HANDLE;
	commandBufferHandler.Submit(info);
}

void PostEffectHandler::RecreateLayerAssets(const uint32_t index)
{
	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& frameBufferHandler = core.GetFrameBufferHandler();
	auto& imageHandler = _renderer.GetImageHandler();
	auto& memoryHandler = _renderer.GetMemoryHandler();
	auto& swapChain = _renderer.GetSwapChain();
	auto& syncHandler = _renderer.GetSyncHandler();

	// Set up the first render pass to use anti aliasing, if enabled and if the device supports it.
	auto msaaSamples = vi::VkCorePhysicalDevice::GetMaxUsableSampleCount(_renderer.GetPhysicalDevice());
	msaaSamples = vi::Ut::Min(_msaaSamples, msaaSamples);

	// Get color and depth image formats.
	const auto format = swapChain.GetFormat();
	const auto depthBufferFormat = swapChain.GetDepthBufferFormat();

	const uint32_t swapChainLength = swapChain.GetLength();
	PostEffect::Frame* start = &GetStartFrame(index);

	for (uint32_t i = 0; i < swapChainLength; ++i)
	{
		auto& frame = start[i];

		frame.descriptorSet = _descriptorPool.Get();
		frame.commandBuffer = commandBufferHandler.Create();
		frame.renderFinishedSemaphore = syncHandler.CreateSemaphore();

		// Create color image for the layer.
		vi::VkImageHandler::CreateInfo colorImageCreateInfo{};
		colorImageCreateInfo.resolution = _extent;
		colorImageCreateInfo.format = format;
		colorImageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (index == 0)
			colorImageCreateInfo.samples = msaaSamples;
		frame.colorImage = imageHandler.Create(colorImageCreateInfo);

		// Create depth image for the layer.
		vi::VkImageHandler::CreateInfo depthImageCreateInfo{};
		depthImageCreateInfo.resolution = _extent;
		depthImageCreateInfo.format = depthBufferFormat;
		depthImageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		frame.depthImage = imageHandler.Create(depthImageCreateInfo);

		frame.colorMemory = memoryHandler.Allocate(frame.colorImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		frame.depthMemory = memoryHandler.Allocate(frame.depthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		memoryHandler.Bind(frame.colorImage, frame.colorMemory);
		memoryHandler.Bind(frame.depthImage, frame.depthMemory);

		vi::VkImageHandler::ViewCreateInfo colorViewCreateInfo{};
		colorViewCreateInfo.image = frame.colorImage;
		colorViewCreateInfo.format = format;
		frame.imageView = imageHandler.CreateView(colorViewCreateInfo);

		vi::VkImageHandler::ViewCreateInfo depthViewCreateInfo{};
		depthViewCreateInfo.image = frame.depthImage;
		depthViewCreateInfo.format = depthBufferFormat;
		depthViewCreateInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		frame.depthImageView = imageHandler.CreateView(depthViewCreateInfo);

		// Create render target.
		vi::VkFrameBufferHandler::CreateInfo frameBufferCreateInfo{};
		frameBufferCreateInfo.imageViews = frame.imageViews;
		frameBufferCreateInfo.imageViewCount = 2;
		frameBufferCreateInfo.renderPass = _renderPass;
		frameBufferCreateInfo.extent = _extent;
		frame.frameBuffer = frameBufferHandler.Create(frameBufferCreateInfo);
	}

	_postEffects[index]->OnRecreateAssets();
}

void PostEffectHandler::DestroyLayerAssets(const uint32_t index, const bool calledByDestructor) const
{
	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& frameBufferHandler = core.GetFrameBufferHandler();
	auto& imageHandler = _renderer.GetImageHandler();
	auto& memoryHandler = _renderer.GetMemoryHandler();
	auto& swapChain = _renderer.GetSwapChain();
	auto& syncHandler = _renderer.GetSyncHandler();

	const uint32_t swapChainLength = swapChain.GetLength();
	PostEffect::Frame* start = &GetStartFrame(index);

	// Destroy the contents in the layers.
	for (uint32_t i = 0; i < swapChainLength; ++i)
	{
		auto& frame = start[i];

		commandBufferHandler.Destroy(frame.commandBuffer);
		syncHandler.DestroySemaphore(frame.renderFinishedSemaphore);

		frameBufferHandler.Destroy(frame.frameBuffer);

		imageHandler.DestroyView(frame.imageView);
		imageHandler.DestroyView(frame.depthImageView);

		imageHandler.Destroy(frame.colorImage);
		imageHandler.Destroy(frame.depthImage);

		memoryHandler.Free(frame.colorMemory);
		memoryHandler.Free(frame.depthMemory);
	}

	if(!calledByDestructor)
		_postEffects[index]->DestroyAssets();
}

void PostEffectHandler::DestroySwapChainAssets(const bool calledByDestructor) const
{
	auto& renderPassHandler = _renderer.GetRenderPassHandler();
	renderPassHandler.Destroy(_renderPass);

	for (uint32_t i = 0; i < _postEffects.GetCount(); ++i)
		DestroyLayerAssets(i, calledByDestructor);
}

PostEffect::Frame& PostEffectHandler::GetStartFrame(const uint32_t index) const
{
	return _frames[_renderer.GetSwapChain().GetLength() * index];
}

PostEffect::Frame& PostEffectHandler::GetActiveFrame(const uint32_t index) const
{
	return (&GetStartFrame(index))[_imageIndex];
}
