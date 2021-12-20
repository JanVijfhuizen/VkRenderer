#include "pch.h"
#include "Rendering/RenderSystem.h"
#include "VkRenderer/WindowHandlerGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "VkRenderer/SwapChain.h"

RenderManager::RenderManager()
{
	_windowHandler = GMEM.New<vi::WindowHandlerGLFW>();

	vi::VkRenderer::Settings settings;
	settings.windowHandler = _windowHandler;
	settings.debugger.validationLayers.push_back("VK_LAYER_RENDERDOC_Capture");
	settings.allocator = &GMEM;
	_vkRenderer = GMEM.New<vi::VkRenderer>(settings);
	_swapChain = &_vkRenderer->GetSwapChain();
}

RenderManager::~RenderManager()
{
	_windowHandler->~WindowHandlerGLFW();
	_vkRenderer->~VkRenderer();
}

void RenderManager::BeginFrame(bool& quit)
{
	_windowHandler->BeginFrame(quit);
	if (quit)
		return;

	_swapChain->BeginFrame();
}

void RenderManager::EndFrame()
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
