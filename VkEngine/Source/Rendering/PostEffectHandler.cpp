#include "pch.h"
#include "Rendering/PostEffectHandler.h"
#include "VkRenderer/VkHandlers/VkRenderPassHandler.h"
#include "Rendering/Renderer.h"

PostEffect::PostEffect(Renderer& renderer) : renderer(renderer)
{

}

BasicPostEffect::BasicPostEffect(Renderer& renderer, const char* shaderName) : PostEffect(renderer)
{
	auto& shaderExt = renderer.GetShaderExt();
	_shader = shaderExt.Load(shaderName);
}

BasicPostEffect::~BasicPostEffect()
{
	DestroyAssets();

	auto& shaderExt = renderer.GetShaderExt();
	shaderExt.DestroyShader(_shader);
}

void BasicPostEffect::Draw(Frame& frame)
{
	auto& descriptorPoolHandler = renderer.GetDescriptorPoolHandler();
	auto& shaderHandler = renderer.GetShaderHandler();
	auto& meshHandler = renderer.GetMeshHandler();
	auto& pipelineHandler = renderer.GetPipelineHandler();
	auto& postEffectHandler = renderer.GetPostEffectHandler();
	auto& swapChainext = renderer.GetSwapChainExt();

	pipelineHandler.Bind(_pipeline, _pipelineLayout);
	meshHandler.Bind(postEffectHandler.GetMesh());

	const auto sampler = shaderHandler.CreateSampler();
	shaderHandler.BindSampler(frame.descriptorSet, frame.imageView, 
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, sampler, 0, 0);

	vi::VkShaderHandler::SamplerCreateInfo depthSamplerCreateInfo{};
	depthSamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	depthSamplerCreateInfo.adressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	const auto depthSampler = shaderHandler.CreateSampler(depthSamplerCreateInfo);
	shaderHandler.BindSampler(frame.descriptorSet, frame.depthImageView, 
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, depthSampler, 1, 0);

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

	vi::VkPipelineHandler::CreateInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.Add(postEffectHandler.GetLayout());
	pipelineInfo.modules.Add(_shader.vertex);
	pipelineInfo.modules.Add(_shader.fragment);
	pipelineInfo.renderPass = swapChain.GetRenderPass();
	pipelineInfo.extent = swapChain.GetExtent();

	pipelineHandler.Create(pipelineInfo, _pipeline, _pipelineLayout);
}

void BasicPostEffect::DestroyAssets()
{
	auto& pipelineHandler = renderer.GetPipelineHandler();
	pipelineHandler.Destroy(_pipeline, _pipelineLayout);
}

PostEffectHandler::PostEffectHandler(Renderer& renderer, const VkSampleCountFlagBits msaaSamples) : 
	VkHandler(renderer), Dependency(renderer), _renderer(renderer), _msaaSamples(msaaSamples)
{
	vi::VkLayoutHandler::CreateInfo layoutInfo{};
	auto& samplerBinding = layoutInfo.bindings.Add();
	samplerBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutInfo.bindings.Add(samplerBinding);
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	VkDescriptorType uboType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uint32_t blockSize = 8 * SWAPCHAIN_MAX_FRAMES;
	_descriptorPool.Construct(_renderer, _layout, &uboType, &blockSize, 1, blockSize / 2);

	_mesh = renderer.GetMeshHandler().Create(MeshHandler::GenerateQuad());

	OnRecreateSwapChainAssets();
}

PostEffectHandler::~PostEffectHandler()
{
	_renderer.GetLayoutHandler().DestroyLayout(_layout);
	_renderer.GetMeshHandler().Destroy(_mesh);
	DestroySwapChainAssets(true);
	_descriptorPool.Cleanup();
}

void PostEffectHandler::BeginFrame()
{
	LayerBeginFrame(0);
}

void PostEffectHandler::EndFrame()
{
	auto& swapChain = core.GetSwapChain();

	_imageIndex = swapChain.GetImageIndex();

	const uint32_t count = _layers.GetCount();
	for (uint32_t i = 0; i < count; ++i)
	{
		auto& layer = _layers[i];
		auto& frame = layer.frames[_imageIndex];

		LayerEndFrame(i);
		if (i < count - 1)
		{
			LayerBeginFrame(i + 1);
			layer.postEffect->Draw(frame);
		}
	}
}

void PostEffectHandler::Draw() const
{
	auto& finalLayer = _layers[_layers.GetCount() - 1];
	finalLayer.postEffect->Draw(finalLayer.frames[_imageIndex]);
}

VkRenderPass PostEffectHandler::GetRenderPass() const
{
	return _renderPass;
}

VkExtent2D PostEffectHandler::GetExtent() const
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
	auto& layer = _layers.Add();
	layer.postEffect = postEffect;
	RecreateLayerAssets(layer);
}

bool PostEffectHandler::IsEmpty() const
{
	return _layers.GetCount() == 0;
}

void PostEffectHandler::OnRecreateSwapChainAssets()
{
	if (_renderPass)
		DestroySwapChainAssets(false);

	auto& renderPassHandler = _renderer.GetRenderPassHandler();
	auto& swapChain = _renderer.GetSwapChain();

	vi::VkRenderPassHandler::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.useColorAttachment = true;
	renderPassCreateInfo.colorFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	renderPassCreateInfo.useDepthAttachment = true;
	renderPassCreateInfo.depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	renderPassCreateInfo.depthFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	_renderPass = renderPassHandler.Create(renderPassCreateInfo);
	_extent = swapChain.GetExtent();

	for (auto& layer : _layers)
		RecreateLayerAssets(layer);
}

void PostEffectHandler::LayerBeginFrame(const uint32_t index)
{
	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& renderPassHandler = core.GetRenderPassHandler();
	auto& swapChain = core.GetSwapChain();

	_imageIndex = swapChain.GetImageIndex();

	auto& firstLayer = _layers[index];
	auto& frame = firstLayer.frames[_imageIndex];

	commandBufferHandler.BeginRecording(frame.commandBuffer);

	VkClearValue clearColors[2];
	clearColors[0].color = { 0, 0, 0, 1 };
	clearColors[1].depthStencil = { 1, 0 };

	renderPassHandler.Begin(frame.frameBuffer, _renderPass, {},
		{ _extent.width, _extent.height }, clearColors, 2);
}

void PostEffectHandler::LayerEndFrame(const uint32_t index) const
{
	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& renderPassHandler = core.GetRenderPassHandler();
	auto& syncHandler = _renderer.GetSyncHandler();

	auto& layer = _layers[index];
	auto& frame = layer.frames[_imageIndex];

	renderPassHandler.End();
	commandBufferHandler.EndRecording();

	vi::VkCommandBufferHandler::SubmitInfo info{};
	info.buffers = &frame.commandBuffer;
	info.buffersCount = 1;
	info.waitSemaphore = VK_NULL_HANDLE;
	info.signalSemaphore = VK_NULL_HANDLE;
	info.fence = frame.fence;
	commandBufferHandler.Submit(info);

	syncHandler.WaitForFence(frame.fence);
}

void PostEffectHandler::RecreateLayerAssets(Layer& layer)
{
	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& frameBufferHandler = core.GetFrameBufferHandler();
	auto& imageHandler = _renderer.GetImageHandler();
	auto& memoryHandler = _renderer.GetMemoryHandler();
	auto& swapChainHandler = _renderer.GetSwapChain();
	auto& syncHandler = _renderer.GetSyncHandler();

	auto msaaMaxSamples = vi::VkCorePhysicalDevice::GetMaxUsableSampleCount(_renderer.GetPhysicalDevice());


	for (auto& frame : layer.frames)
	{
		frame.descriptorSet = _descriptorPool.Get();
		frame.commandBuffer = commandBufferHandler.Create();
		frame.fence = syncHandler.CreateFence();

		vi::VkImageHandler::CreateInfo colorImageCreateInfo{};
		colorImageCreateInfo.resolution = { _extent.width, _extent.height };
		colorImageCreateInfo.format = swapChainHandler.GetFormat();
		colorImageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		frame.colorImage = imageHandler.Create(colorImageCreateInfo);

		vi::VkImageHandler::CreateInfo depthImageCreateInfo{};
		depthImageCreateInfo.resolution = { _extent.width, _extent.height };
		depthImageCreateInfo.format = swapChainHandler.GetDepthBufferFormat();
		depthImageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		frame.depthImage = imageHandler.Create(depthImageCreateInfo);

		frame.colorMemory = memoryHandler.Allocate(frame.colorImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		frame.depthMemory = memoryHandler.Allocate(frame.depthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		memoryHandler.Bind(frame.colorImage, frame.colorMemory);
		memoryHandler.Bind(frame.depthImage, frame.depthMemory);

		vi::VkImageHandler::ViewCreateInfo colorViewCreateInfo{};
		colorViewCreateInfo.image = frame.colorImage;
		colorViewCreateInfo.format = swapChainHandler.GetFormat();
		frame.imageView = imageHandler.CreateView(colorViewCreateInfo);

		vi::VkImageHandler::ViewCreateInfo depthViewCreateInfo{};
		depthViewCreateInfo.image = frame.depthImage;
		depthViewCreateInfo.format = swapChainHandler.GetDepthBufferFormat();
		depthViewCreateInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		frame.depthImageView = imageHandler.CreateView(depthViewCreateInfo);

		vi::VkFrameBufferHandler::CreateInfo frameBufferCreateInfo{};
		frameBufferCreateInfo.imageViews = frame.imageViews;
		frameBufferCreateInfo.imageViewCount = 2;
		frameBufferCreateInfo.renderPass = _renderPass;
		frameBufferCreateInfo.extent = _extent;
		frame.frameBuffer = frameBufferHandler.Create(frameBufferCreateInfo);
	}

	layer.postEffect->OnRecreateAssets();
}

void PostEffectHandler::DestroyLayerAssets(Layer& layer, const bool calledByDestructor) const
{
	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& frameBufferHandler = core.GetFrameBufferHandler();
	auto& imageHandler = _renderer.GetImageHandler();
	auto& memoryHandler = _renderer.GetMemoryHandler();
	auto& syncHandler = _renderer.GetSyncHandler();

	for (auto& frame : layer.frames)
	{
		commandBufferHandler.Destroy(frame.commandBuffer);
		syncHandler.DestroyFence(frame.fence);

		frameBufferHandler.Destroy(frame.frameBuffer);

		imageHandler.DestroyView(frame.imageView);
		imageHandler.DestroyView(frame.depthImageView);

		imageHandler.Destroy(frame.colorImage);
		imageHandler.Destroy(frame.depthImage);

		memoryHandler.Free(frame.colorMemory);
		memoryHandler.Free(frame.depthMemory);
	}

	if(!calledByDestructor)
		layer.postEffect->DestroyAssets();
}

void PostEffectHandler::DestroySwapChainAssets(const bool calledByDestructor) const
{
	auto& renderPassHandler = _renderer.GetRenderPassHandler();
	renderPassHandler.Destroy(_renderPass);

	for (auto& layer : _layers)
		DestroyLayerAssets(layer, calledByDestructor);
}
