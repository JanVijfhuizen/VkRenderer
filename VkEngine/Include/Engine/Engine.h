#pragma once
#include "VkRenderer/WindowHandlerGLFW.h"
#include "Components/Material.h"
#include "Components/Transform.h"
#include "Rendering/Renderer.h"
#include "Components/Camera.h"
#include "Components/Light.h"

template <typename GameState>
class Engine final
{
public:
	struct Info final
	{
		size_t capacity = 1e2f;
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

	[[nodiscard]] Renderer& GetRenderer() const;
	[[nodiscard]] ce::Cecsar& GetCecsar() const;

	[[nodiscard]] CameraSystem& GetCameras() const;
	[[nodiscard]] LightSystem& GetLights() const;
	[[nodiscard]] MaterialSystem& GetMaterials() const;
	[[nodiscard]] TransformSystem& GetTransforms() const;
	[[nodiscard]] ShadowCasterSystem& GetShadowCasters() const;

private:
	bool _isRunning = false;
	vi::WindowHandlerGLFW* _windowHandler;
	Renderer* _renderer;
	ce::Cecsar* _cecsar;

	TransformSystem* _transforms;
	CameraSystem* _cameras;
	MaterialSystem* _materials;
	ShadowCasterSystem* _shadowCasterSystem;
	LightSystem* _lightSystem;

	GameState* _gameState;
	BasicPostEffect* _defaultPostEffect = nullptr;
};

template <typename GameState>
int Engine<GameState>::Run(const Info& info)
{
	assert(!_isRunning);
	_isRunning = true;

	_windowHandler = GMEM.New<vi::WindowHandlerGLFW>();

	{
		vi::VkCoreInfo vkInfo{};
		Renderer::Info addInfo{};
		addInfo.msaaSamples = info.msaaSamples;
		vkInfo.windowHandler = _windowHandler;
		if(info.useRenderDoc)
			vkInfo.validationLayers.Add("VK_LAYER_RENDERDOC_Capture");
		_renderer = GMEM.New<Renderer>(vkInfo, addInfo);
	}

	_cecsar = GMEM.New<ce::Cecsar>(info.capacity);
	_transforms = GMEM.New<TransformSystem>(*_cecsar);
	_cameras = GMEM.New<CameraSystem>(*_cecsar, *_renderer, *_transforms);
	_materials = GMEM.New<MaterialSystem>(*_cecsar, *_renderer, *_transforms, *_cameras, "");
	_shadowCasterSystem = GMEM.New<ShadowCasterSystem>(*_cecsar);
	_lightSystem = GMEM.New<LightSystem>(*_cecsar, *_renderer, *_materials, *_shadowCasterSystem, *_transforms);

	_gameState = GMEM.New<GameState>();

	auto& postEffectHandler = _renderer->GetPostEffectHandler();
	auto& swapChain = _renderer->GetSwapChain();
	auto& swapChainExt = _renderer->GetSwapChainExt();

	if (info.awake)
		info.awake(*this, *_gameState);
	if (info.start)
		info.start(*this, *_gameState);

	if(postEffectHandler.IsEmpty())
	{
		_defaultPostEffect = GMEM.New<BasicPostEffect>(*_renderer, "post-");
		postEffectHandler.Add(_defaultPostEffect);
	}

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

		_lightSystem->Render(swapChain.GetImageAvaiableSemaphore());
		postEffectHandler.BeginFrame(_lightSystem->GetRenderFinishedSemaphore());

		_cameras->Update();
		_materials->Draw();

		if (info.renderUpdate)
			info.renderUpdate(*this, *_gameState, outQuit);
		if (outQuit)
			break;

		postEffectHandler.EndFrame();
		swapChain.BeginFrame(false);
		postEffectHandler.Draw();
		
		swapChain.EndFrame(postEffectHandler.GetRenderFinishedSemaphore());
		swapChainExt.Update();
	}

	_renderer->DeviceWaitIdle();

	if (info.cleanup)
		info.cleanup(*this, *_gameState);

	GMEM.Delete(_gameState);
	GMEM.Delete(_lightSystem);
	GMEM.Delete(_shadowCasterSystem);
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
Renderer& Engine<GameState>::GetRenderer() const
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
	return *_lightSystem;
}

template <typename GameState>
ShadowCasterSystem& Engine<GameState>::GetShadowCasters() const
{
	return *_shadowCasterSystem;
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
