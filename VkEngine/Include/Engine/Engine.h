#pragma once
#include "VkRenderer/WindowHandlerGLFW.h"
#include "Components/Material.h"
#include "Components/Transform.h"
#include "Rendering/VulkanRenderer.h"
#include "Components/Camera.h"
#include "Components/Light.h"
#include "VkRenderer/VkCore/VkCoreInfo.h"
#include "VkRenderer/VkCore/VkCoreSwapchain.h"
#include "Rendering/PostEffectHandler.h"
#include "Components/Renderer.h"

/// <summary>
/// An engine specifically made for a single game.
/// </summary>
/// <typeparam name="GameState"></typeparam>
template <typename GameState>
class Engine final
{
public:
	struct Info final
	{
		// Maximum amount of entities in the scene.
		size_t capacity = 1e2f;
		// Debug using render doc.
		bool useRenderDoc = false;
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		typedef void (*Awake)(Engine& engine, GameState& gameState);
		typedef void (*Start)(Engine& engine, GameState& gameState);
		typedef void (*Update)(Engine& engine, GameState& gameState, bool& outQuit);
		typedef void (*Cleanup)(Engine& engine, GameState& gameState);

		Awake awake = nullptr;
		Start start = nullptr;
		Update update = nullptr;
		Update physicsUpdate = nullptr;
		Update preRenderUpdate = nullptr;
		Update renderUpdate = nullptr;
		Cleanup cleanup = nullptr;
	};

	[[nodiscard]] int Run(const Info& info);
	[[nodiscard]] bool IsRunning() const;

	[[nodiscard]] VulkanRenderer& GetVulkanRenderer() const;
	[[nodiscard]] ce::Cecsar& GetCecsar() const;

	[[nodiscard]] CameraSystem& GetCameras() const;
	[[nodiscard]] LightSystem& GetLights() const;
	[[nodiscard]] MaterialSystem& GetMaterials() const;
	[[nodiscard]] RenderSystem& GetRenderers() const;
	[[nodiscard]] TransformSystem& GetTransforms() const;
	[[nodiscard]] ShadowCasterSystem& GetShadowCasters() const;

private:
	bool _isRunning = false;
	vi::WindowHandlerGLFW* _windowHandler;
	VulkanRenderer* _renderer;
	ce::Cecsar* _cecsar;

	CameraSystem* _cameras;
	LightSystem* _lights;
	MaterialSystem* _materials;
	RenderSystem* _renderers;
	ShadowCasterSystem* _shadowCasters;
	TransformSystem* _transforms;

	GameState* _gameState;
	BasicPostEffect* _defaultPostEffect = nullptr;
};

template <typename GameState>
int Engine<GameState>::Run(const Info& info)
{
	assert(!_isRunning);
	_isRunning = true;

	_windowHandler = GMEM.New<vi::WindowHandlerGLFW>();

	// Create the vulkan renderer.
	{
		vi::VkCoreInfo vkInfo{};
		VulkanRenderer::Info addInfo{};
		addInfo.msaaSamples = info.msaaSamples;
		vkInfo.windowHandler = _windowHandler;
		if(info.useRenderDoc)
			vkInfo.validationLayers.Add("VK_LAYER_RENDERDOC_Capture");
		_renderer = GMEM.New<VulkanRenderer>(vkInfo, addInfo);
	}

	_cecsar = GMEM.New<ce::Cecsar>(info.capacity);
	_transforms = GMEM.New<TransformSystem>(*_cecsar);
	_cameras = GMEM.New<CameraSystem>(*_cecsar, *_renderer, *_transforms);
	_materials = GMEM.New<MaterialSystem>(*_cecsar, *_renderer);
	_shadowCasters = GMEM.New<ShadowCasterSystem>(*_cecsar);
	_lights = GMEM.New<LightSystem>(*_cecsar, *_renderer, *_materials, *_shadowCasters, *_transforms);
	_renderers = GMEM.New<RenderSystem>(*_cecsar, *_renderer, *_materials, *_cameras, *_lights, *_transforms);

	_gameState = GMEM.New<GameState>();

	auto& postEffectHandler = _renderer->GetPostEffectHandler();
	auto& swapChain = _renderer->GetSwapChain();
	auto& swapChainExt = _renderer->GetSwapChainExt();

	if (info.awake)
		info.awake(*this, *_gameState);
	if (info.start)
		info.start(*this, *_gameState);

	// The post effect handler needs at least one post effect to be able to use MSAA.
	if(postEffectHandler.IsEmpty())
	{
		_defaultPostEffect = GMEM.New<BasicPostEffect>(*_renderer, "post-");
		postEffectHandler.Add(_defaultPostEffect);
	}

	// Game update.
	while (true)
	{
		bool outQuit = false;
		_windowHandler->BeginFrame(outQuit);
		if (outQuit)
			break;

		if (info.update)
			info.update(*this, *_gameState, outQuit);
		if (outQuit)
			break;
		if (info.physicsUpdate)
			info.physicsUpdate(*this, *_gameState, outQuit);
		if (outQuit)
			break;

		if (info.preRenderUpdate)
			info.preRenderUpdate(*this, *_gameState, outQuit);
		if (outQuit)
			break;

		swapChain.WaitForImage();

		// Render the lights before rendering anything else, since they might want to use the lightmaps.
		_lights->Render(swapChain.GetImageAvaiableSemaphore());
		// Render the scene to the first post effect layer.
		postEffectHandler.BeginFrame(_lights->GetRenderFinishedSemaphore());

		_cameras->Update();
		_renderers->Draw();

		if (info.renderUpdate)
			info.renderUpdate(*this, *_gameState, outQuit);
		if (outQuit)
			break;

		// End the post effect rendering and render the result to the swapchain.
		postEffectHandler.EndFrame();
		swapChain.BeginFrame(false);
		postEffectHandler.Render();
		
		swapChain.EndFrame(postEffectHandler.GetRenderFinishedSemaphore());
		swapChainExt.Update();
	}

	_renderer->DeviceWaitIdle();

	if (info.cleanup)
		info.cleanup(*this, *_gameState);

	GMEM.Delete(_gameState);
	GMEM.Delete(_renderers);
	GMEM.Delete(_lights);
	GMEM.Delete(_shadowCasters);
	GMEM.Delete(_defaultPostEffect);
	GMEM.Delete(_materials);
	GMEM.Delete(_cameras);
	GMEM.Delete(_transforms);
	GMEM.Delete(_renderer);
	GMEM.Delete(_cecsar);
	GMEM.Delete(_windowHandler);

	_isRunning = false;
	return EXIT_SUCCESS;
}

template <typename GameState>
bool Engine<GameState>::IsRunning() const
{
	return _isRunning;
}

template <typename GameState>
VulkanRenderer& Engine<GameState>::GetVulkanRenderer() const
{
	return *_renderer;
}

template <typename GameState>
ce::Cecsar& Engine<GameState>::GetCecsar() const
{
	return *_cecsar;
}

template <typename GameState>
CameraSystem& Engine<GameState>::GetCameras() const
{
	return *_cameras;
}

template <typename GameState>
LightSystem& Engine<GameState>::GetLights() const
{
	return *_lights;
}

template <typename GameState>
ShadowCasterSystem& Engine<GameState>::GetShadowCasters() const
{
	return *_shadowCasters;
}

template <typename GameState>
TransformSystem& Engine<GameState>::GetTransforms() const
{
	return *_transforms;
}

template <typename GameState>
MaterialSystem& Engine<GameState>::GetMaterials() const
{
	return *_materials;
}

template <typename GameState>
RenderSystem& Engine<GameState>::GetRenderers() const
{
	return *_renderers;
}
