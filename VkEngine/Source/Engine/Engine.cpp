#include "pch.h"
#include "Engine/Engine.h"

int Engine::Run(const Info& info)
{
	assert(!_isRunning);
	_isRunning = true;

	_windowHandler = { GMEM };

	{
		vi::VkCoreInfo vkInfo{};
		vkInfo.windowHandler = _windowHandler;
		vkInfo.validationLayers.Add("VK_LAYER_RENDERDOC_Capture");
		_renderer = { GMEM, vkInfo };
	}

	_cecsar = { GMEM, info.capacity };
	_transforms = { GMEM, *_cecsar };
	_materials = { GMEM, *_cecsar, *_renderer, "" };

	if (info.awake)
		info.awake();
	if (info.start)
		info.start();

	auto& swapChain = _renderer->GetSwapChain();
	while (true)
	{
		bool outQuit = false;
		_windowHandler->BeginFrame(outQuit);
		if (outQuit)
			break;

		if(info.preRenderUpdate)
			info.preRenderUpdate(outQuit);
		if (outQuit)
			break;

		swapChain.BeginFrame();

		_materials->Update();

		if(info.renderUpdate)
			info.renderUpdate(outQuit);
		if (outQuit)
			break;

		bool recreateAssets = false;
		swapChain.EndFrame(recreateAssets);	

		if (info.update)
			info.update(outQuit);
		if (outQuit)
			break;
		if (info.physicsUpdate)
			info.physicsUpdate(outQuit);
		if (outQuit)
			break;
	}

	_isRunning = false;
	return EXIT_SUCCESS;
}

bool Engine::IsRunning() const
{
	return _isRunning;
}

ce::Cecsar& Engine::GetCecsar() const
{
	return *_cecsar;
}

Renderer& Engine::GetRenderer() const
{
	return *_renderer;
}

TransformSystem& Engine::GetTransforms() const
{
	return *_transforms;
}

MaterialSystem& Engine::GetMaterials() const
{
	return *_materials;
}
