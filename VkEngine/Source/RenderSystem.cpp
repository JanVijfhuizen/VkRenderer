#include "pch.h"
#include "RenderSystem.h"
#include "VkRenderer/WindowHandlerGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "VkRenderer/SwapChain.h"

RenderSystem::RenderSystem()
{
	_windowHandler = new vi::WindowHandlerGLFW;

	vi::VkRenderer::Settings settings;
	settings.windowHandler = _windowHandler;
	settings.debugger.validationLayers.push_back("VK_LAYER_RENDERDOC_Capture");
	_vkRenderer = new vi::VkRenderer{settings};
	_swapChain = &_vkRenderer->GetSwapChain();
}

RenderSystem::~RenderSystem()
{
	delete _vkRenderer;
	delete _windowHandler;
}

void RenderSystem::BeginFrame(bool& quit)
{
	_windowHandler->BeginFrame(quit);
	if (quit)
		return;

	_swapChain->BeginFrame();
}

void RenderSystem::EndFrame()
{
	bool shouldRecreateAssets;
	_swapChain->EndFrame(shouldRecreateAssets);
}

vi::WindowHandlerGLFW& RenderSystem::GetWindowHandler() const
{
	return *_windowHandler;
}

vi::VkRenderer& RenderSystem::GetVkRenderer() const
{
	return *_vkRenderer;
}