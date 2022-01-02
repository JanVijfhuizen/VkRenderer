#include "pch.h"
#include "Rendering/RenderManager.h"
#include "VkRenderer/WindowHandlerGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "VkRenderer/SwapChain.h"

RenderManager::RenderManager()
{
	_windowHandler = GMEM.New<vi::WindowHandlerGLFW>();

	vi::VkRenderer::Settings settings;
	settings.windowHandler = _windowHandler;
	settings.debugger.validationLayers.push_back("VK_LAYER_RENDERDOC_Capture");
	_vkRenderer = GMEM.New<vi::VkRenderer>(settings);
	_swapChain = &_vkRenderer->GetSwapChain();
}

RenderManager::~RenderManager()
{
	GMEM.Delete(_windowHandler);
	GMEM.Delete(_vkRenderer);
}

void RenderManager::BeginFrame(bool& quit, const bool callSwapChainBeginFrame)
{
	_windowHandler->BeginFrame(quit);
	if (quit)
		return;

	if(callSwapChainBeginFrame)
		_swapChain->BeginFrame();
}

void RenderManager::EndFrame() const
{
	bool shouldRecreateAssets;
	_swapChain->EndFrame(shouldRecreateAssets);

	if (shouldRecreateAssets)
		return;
}

vi::WindowHandlerGLFW& RenderManager::GetWindowHandler() const
{
	return *_windowHandler;
}

vi::VkRenderer& RenderManager::GetVkRenderer() const
{
	return *_vkRenderer;
}
