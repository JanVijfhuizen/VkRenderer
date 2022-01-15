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
		//vkInfo.validationLayers.Add("VK_LAYER_RENDERDOC_Capture");
		_renderer = { GMEM, vkInfo };
	}

	_cecsar = { GMEM, info.capacity };
	_transforms = { GMEM, *_cecsar };
	_cameras = { GMEM, *_cecsar, *_renderer, *_transforms };
	_materials = { GMEM, *_cecsar, *_renderer, *_transforms, *_cameras, "" };

	auto& postEffectHandler = _renderer->GetPostEffectHandler();
	auto& swapChain = _renderer->GetSwapChain();
	auto& swapChainExt = _renderer->GetSwapChainExt();

	const auto msaa = GMEM.New<MSAA>(*_renderer);
	postEffectHandler.Add(msaa);

	if (info.awake)
		info.awake(*this);
	if (info.start)
		info.start(*this);

	while (true)
	{
		bool outQuit = false;
		_windowHandler->BeginFrame(outQuit);
		if (outQuit)
			break;

		if (info.update)
			info.update(*this, outQuit);
		if (outQuit)
			break;
		if (info.physicsUpdate)
			info.physicsUpdate(*this, outQuit);
		if (outQuit)
			break;

		if(info.preRenderUpdate)
			info.preRenderUpdate(*this, outQuit);
		if (outQuit)
			break;

		swapChain.WaitForImage();
		postEffectHandler.BeginFrame();

		_cameras->Update();
		_materials->Update();

		if (info.renderUpdate)
			info.renderUpdate(*this, outQuit);
		if (outQuit)
			break;

		postEffectHandler.EndFrame();

		swapChain.BeginFrame(false);
		postEffectHandler.Draw();
		swapChain.EndFrame();

		swapChainExt.Update();
	}

	_renderer->DeviceWaitIdle();
	GMEM.Delete(msaa);

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

CameraSystem& Engine::GetCameras() const
{
	return *_cameras;
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
