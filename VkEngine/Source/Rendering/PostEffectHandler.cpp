#include "pch.h"
#include "Rendering/PostEffectHandler.h"
#include "VkRenderer/VkHandlers/VkRenderPassHandler.h"
#include "Rendering/Renderer.h"

PostEffectHandler::PostEffectHandler(Renderer& renderer) : VkHandler(renderer), Dependency(renderer), _renderer(renderer)
{
	auto& shaderExt = renderer.GetShaderExt();
	_shader = shaderExt.Load("post-");

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
	DestroySwapChainAssets();
	_descriptorPool.Cleanup();

	auto& shaderExt = _renderer.GetShaderExt();
	shaderExt.DestroyShader(_shader);
}

void PostEffectHandler::BeginFrame()
{
	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& renderPassHandler = core.GetRenderPassHandler();
	auto& swapChain = core.GetSwapChain();

	_imageIndex = swapChain.GetImageIndex();
	auto& frame = _frames[_imageIndex];

	commandBufferHandler.BeginRecording(frame.commandBuffer);

	VkClearValue clearColors[2];
	clearColors[0].color = {0, 0, 0, 1};
	clearColors[1].depthStencil = {1, 0};

	renderPassHandler.Begin(frame.frameBuffer, _renderPass, {},
		{_extent.width, _extent.height}, clearColors, 2);
}

void PostEffectHandler::EndFrame()
{
	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& renderPassHandler = core.GetRenderPassHandler();
	auto& syncHandler = _renderer.GetSyncHandler();

	auto& frame = _frames[_imageIndex];

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

void PostEffectHandler::Draw()
{
	auto& descriptorPoolHandler = _renderer.GetDescriptorPoolHandler();
	auto& shaderHandler = _renderer.GetShaderHandler();
	auto& meshHandler = _renderer.GetMeshHandler();
	auto& pipelineHandler = _renderer.GetPipelineHandler();
	auto& swapChainext = _renderer.GetSwapChainExt();

	auto& frame = _frames[_imageIndex];

	pipelineHandler.Bind(_pipeline, _pipelineLayout);
	meshHandler.Bind(_mesh);

	const auto sampler = shaderHandler.CreateSampler();
	shaderHandler.BindSampler(frame.descriptorSet, frame.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, sampler, 0, 0);

	vi::VkShaderHandler::SamplerCreateInfo depthSamplerCreateInfo{};
	depthSamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	depthSamplerCreateInfo.adressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	const auto depthSampler = shaderHandler.CreateSampler(depthSamplerCreateInfo);
	shaderHandler.BindSampler(frame.descriptorSet, frame.depthImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, depthSampler, 1, 0);

	descriptorPoolHandler.BindSets(&frame.descriptorSet, 1);
	meshHandler.Draw();

	swapChainext.Collect(sampler);
	swapChainext.Collect(depthSampler);
}

VkRenderPass PostEffectHandler::GetRenderPass() const
{
	return _renderPass;
}

VkExtent2D PostEffectHandler::GetExtent() const
{
	return _extent;
}

void PostEffectHandler::OnRecreateSwapChainAssets()
{
	if (_renderPass)
		DestroySwapChainAssets();

	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& frameBufferHandler = core.GetFrameBufferHandler();
	auto& imageHandler = _renderer.GetImageHandler();
	auto& memoryHandler = _renderer.GetMemoryHandler();
	auto& renderPassHandler = _renderer.GetRenderPassHandler();
	auto& swapChainHandler = _renderer.GetSwapChain();
	auto& syncHandler = _renderer.GetSyncHandler();

	vi::VkRenderPassHandler::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.useColorAttachment = true;
	renderPassCreateInfo.colorFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	renderPassCreateInfo.useDepthAttachment = true;
	renderPassCreateInfo.depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	renderPassCreateInfo.depthFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	_renderPass = renderPassHandler.Create(renderPassCreateInfo);
	_extent = swapChainHandler.GetExtent();

	for (auto& frame : _frames)
	{
		frame.descriptorSet = _descriptorPool.Get();
		frame.commandBuffer = commandBufferHandler.Create();
		frame.fence = syncHandler.CreateFence();

		vi::VkImageHandler::CreateInfo colorImageCreateInfo{};
		colorImageCreateInfo.resolution = { _extent.width, _extent.height };
		colorImageCreateInfo.format = swapChainHandler.GetFormat();
		colorImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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

	auto& swapChain = _renderer.GetSwapChain();
	auto& pipelineHandler = _renderer.GetPipelineHandler();

	vi::VkPipelineHandler::CreateInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.Add(_layout);
	pipelineInfo.modules.Add(_shader.vertex);
	pipelineInfo.modules.Add(_shader.fragment);
	pipelineInfo.renderPass = swapChain.GetRenderPass();
	pipelineInfo.extent = swapChain.GetExtent();

	pipelineHandler.Create(pipelineInfo, _pipeline, _pipelineLayout);
}

void PostEffectHandler::DestroySwapChainAssets() const
{
	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& frameBufferHandler = core.GetFrameBufferHandler();
	auto& imageHandler = _renderer.GetImageHandler();
	auto& memoryHandler = _renderer.GetMemoryHandler();
	auto& renderPassHandler = _renderer.GetRenderPassHandler();
	auto& syncHandler = _renderer.GetSyncHandler();

	renderPassHandler.Destroy(_renderPass);

	for (auto& frame : _frames)
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

	auto& pipelineHandler = _renderer.GetPipelineHandler();
	pipelineHandler.Destroy(_pipeline, _pipelineLayout);
}
