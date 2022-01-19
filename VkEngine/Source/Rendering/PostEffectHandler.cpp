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
	auto& swapChain = renderer.GetSwapChain();

	vi::VkLayoutHandler::CreateInfo layoutInfo{};
	auto& samplerBinding = layoutInfo.bindings.Add();
	samplerBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutInfo.bindings.Add(samplerBinding);
	_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);

	VkDescriptorType uboType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uint32_t blockSize = 8 * swapChain.GetLength();
	_descriptorPool.Construct(_renderer, _layout, &uboType, &blockSize, 1, blockSize / 2);

	_mesh = renderer.GetMeshHandler().Create(MeshHandler::GenerateQuad());
	_frames.Reallocate(_postEffects.GetLength() * swapChain.GetLength(), GMEM_VOL);

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

	const uint32_t swapChainLength = swapChain.GetLength();
	const uint32_t count = _postEffects.GetCount();

	for (uint32_t i = 0; i < count; ++i)
	{
		auto& postEffect = _postEffects[i];
		auto& frame = _frames[swapChainLength * i + _imageIndex];

		LayerEndFrame(i);
		if(i < count - 1)
		{
			LayerBeginFrame(i + 1);
			postEffect->Draw(frame);
		}
	}
}

void PostEffectHandler::Draw() const
{
	const uint32_t index = _postEffects.GetCount() - 1;
	auto& finalLayer = _postEffects[index];
	finalLayer->Draw(GetActiveFrame(index));
}

VkSemaphore PostEffectHandler::GetRenderFinishedSemaphore() const
{
	const uint32_t index = _postEffects.GetCount() - 1;
	return GetActiveFrame(index).renderFinishedSemaphore;
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

	for (uint32_t i = 0; i < _postEffects.GetCount(); ++i)
		RecreateLayerAssets(i);
}

void PostEffectHandler::LayerBeginFrame(const uint32_t index)
{
	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& renderPassHandler = core.GetRenderPassHandler();
	auto& swapChain = core.GetSwapChain();

	_imageIndex = swapChain.GetImageIndex();

	auto& frame = GetActiveFrame(index);
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
	auto& swapChain = core.GetSwapChain();
	const auto imageAvailableSemaphore = swapChain.GetImageAvaiableSemaphore();

	auto& frame = GetActiveFrame(index);

	renderPassHandler.End();
	commandBufferHandler.EndRecording();

	vi::VkCommandBufferHandler::SubmitInfo info{};
	info.buffers = &frame.commandBuffer;
	info.buffersCount = 1;
	info.waitSemaphore = index == 0 ? imageAvailableSemaphore : GetActiveFrame(index -1).renderFinishedSemaphore;
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

	auto msaaSamples = vi::VkCorePhysicalDevice::GetMaxUsableSampleCount(_renderer.GetPhysicalDevice());
	msaaSamples = vi::Ut::Min(_msaaSamples, msaaSamples);
	
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

		vi::VkImageHandler::CreateInfo colorImageCreateInfo{};
		colorImageCreateInfo.resolution = { _extent.width, _extent.height };
		colorImageCreateInfo.format = format;
		colorImageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (index == 0)
			colorImageCreateInfo.samples = msaaSamples;
		frame.colorImage = imageHandler.Create(colorImageCreateInfo);

		vi::VkImageHandler::CreateInfo depthImageCreateInfo{};
		depthImageCreateInfo.resolution = { _extent.width, _extent.height };
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

PostEffect::Frame& PostEffectHandler::GetStartFrame(uint32_t index) const
{
	return _frames[_renderer.GetSwapChain().GetLength() * index];
}

PostEffect::Frame& PostEffectHandler::GetActiveFrame(const uint32_t index) const
{
	return (&GetStartFrame(index))[_imageIndex];
}
