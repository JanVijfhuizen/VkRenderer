#include "pch.h"
#include "Components/Light.h"
#include "VkRenderer/VkHandlers/VkLayoutHandler.h"
#include "Rendering/Renderer.h"
#include "Components/Camera.h"
#include "Components/Material.h"
#include "Components/Transform.h"

LightSystem::LightSystem(ce::Cecsar& cecsar, Renderer& renderer, CameraSystem& cameras, MaterialSystem& materials,
	ShadowCasterSystem& shadowCasters, TransformSystem& transforms, const Info& info) :
	SmallSystem<Light>(cecsar, info.size), Dependency(renderer),
	_cameras(cameras), _materials(materials), _shadowCasters(shadowCasters), _transforms(transforms)
{
	auto& swapChain = renderer.GetSwapChain();

	//_shader = renderer.GetShaderExt().Load("light-");

	vi::VkLayoutHandler::CreateInfo layoutInfo{};
	//_layout = renderer.GetLayoutHandler().CreateLayout(layoutInfo);
	/*
	VkDescriptorType types = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uint32_t sizes = 32 * swapChain.GetLength();

	_descriptorPool.Construct(renderer, _layout, &types, &sizes, 1, sizes);
	*/

	CreateCubeMaps(renderer.GetSwapChain(), info.shadowResolution);
	OnRecreateSwapChainAssets();
}

LightSystem::~LightSystem()
{
	/*
	_descriptorPool.Cleanup();
	renderer.GetShaderExt().DestroyShader(_shader);
	renderer.GetLayoutHandler().DestroyLayout(_layout);
	*/

	DestroyCubeMaps();
	DestroySwapChainAssets();
}

void LightSystem::Draw()
{
	
}

void LightSystem::CreateCubeMaps(vi::VkCoreSwapchain& swapChain, const glm::ivec2 resolution)
{
	auto& commandBufferHandler = renderer.GetCommandBufferHandler();
	auto& imageHandler = renderer.GetImageHandler();
	auto& memoryHandler = renderer.GetMemoryHandler();
	auto& syncHandler = renderer.GetSyncHandler();

	// Create image with 6 sides.
	vi::VkImageHandler::CreateInfo imageCreateInfo{};
	imageCreateInfo.resolution = resolution;
	imageCreateInfo.arrayLayers = 6;
	imageCreateInfo.format = swapChain.GetDepthBufferFormat();
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	// Create image view.
	vi::VkImageHandler::ViewCreateInfo viewCreateInfo{};
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewCreateInfo.layerCount = 6;
	viewCreateInfo.format = swapChain.GetDepthBufferFormat();
	viewCreateInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

	// Create transition info.
	vi::VkImageHandler::TransitionInfo transitionInfo{};
	transitionInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	transitionInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	transitionInfo.layerCount = 6;

	// Start transition.
	auto cmdBuffer = commandBufferHandler.Create();
	commandBufferHandler.BeginRecording(cmdBuffer);

	// Use the create infos to create a cubemap for every swapchain image.
	_cubeMaps.Reallocate(swapChain.GetLength(), GMEM);
	for (uint32_t i = 0; i < swapChain.GetLength(); ++i)
	{
		auto& cubeMap = _cubeMaps[i];
		cubeMap.image = imageHandler.Create(imageCreateInfo);
		cubeMap.memory = memoryHandler.Allocate(cubeMap.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		memoryHandler.Bind(cubeMap.image, cubeMap.memory);

		viewCreateInfo.image = cubeMap.image;
		cubeMap.view = imageHandler.CreateView(viewCreateInfo);

		transitionInfo.image = cubeMap.image;
		imageHandler.TransitionLayout(transitionInfo);
	}

	const auto fence = syncHandler.CreateFence();

	vi::VkCommandBufferHandler::SubmitInfo submitInfo{};
	submitInfo.buffers = &cmdBuffer;
	submitInfo.buffersCount = 1;
	submitInfo.fence = fence;

	// End recording and execute transitions.
	commandBufferHandler.EndRecording();
	commandBufferHandler.Submit(submitInfo);
	syncHandler.WaitForFence(fence);

	// Destroy command buffer and fence.
	commandBufferHandler.Destroy(cmdBuffer);
	syncHandler.DestroyFence(fence);
}

void LightSystem::DestroyCubeMaps()
{
	auto& imageHandler = renderer.GetImageHandler();
	auto& memoryHandler = renderer.GetMemoryHandler();

	for (auto& cubeMap : _cubeMaps)
	{
		imageHandler.DestroyView(cubeMap.view);
		imageHandler.Destroy(cubeMap.image);
		memoryHandler.Free(cubeMap.memory);
	}
}

void LightSystem::OnRecreateSwapChainAssets()
{
	if (_pipeline)
		DestroySwapChainAssets();
	/*
	auto& postEffectHandler = renderer.GetPostEffectHandler();

	vi::VkPipelineHandler::CreateInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = ShadowVertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = ShadowVertex::GetBindingDescription();
	pipelineInfo.setLayouts.Add(_cameras.GetLayout());
	pipelineInfo.setLayouts.Add(_layout);
	pipelineInfo.modules.Add(_shader.vertex);
	pipelineInfo.modules.Add(_shader.fragment);
	pipelineInfo.pushConstants.Add({ sizeof(Ubo), VK_SHADER_STAGE_VERTEX_BIT });
	pipelineInfo.renderPass = postEffectHandler.GetRenderPass();
	pipelineInfo.extent = postEffectHandler.GetExtent();

	renderer.GetPipelineHandler().Create(pipelineInfo, _pipeline, _pipelineLayout);
	*/
}

void LightSystem::DestroySwapChainAssets() const
{
	//renderer.GetPipelineHandler().Destroy(_pipeline, _pipelineLayout);
}

ShadowCasterSystem::ShadowCasterSystem(ce::Cecsar& cecsar) : System<ShadowCaster>(cecsar)
{
}
