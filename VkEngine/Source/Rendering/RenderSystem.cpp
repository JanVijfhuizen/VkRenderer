﻿#include "pch.h"
#include "Rendering/RenderSystem.h"
#include "VkRenderer/WindowHandlerGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "VkRenderer/SwapChain.h"
#include "DefaultAllocator.h"

RenderSystem::RenderSystem()
{
	_windowHandler = _allocator.Alloc<vi::WindowHandlerGLFW>();
	new (_windowHandler) vi::WindowHandlerGLFW();

	vi::VkRenderer::Settings settings;
	settings.windowHandler = _windowHandler;
	settings.debugger.validationLayers.push_back("VK_LAYER_RENDERDOC_Capture");
	settings.allocator = &_allocator;
	_vkRenderer = _allocator.Alloc<vi::VkRenderer>();
	new (_vkRenderer) vi::VkRenderer(settings);
	_swapChain = &_vkRenderer->GetSwapChain();
}

RenderSystem::~RenderSystem()
{
	_windowHandler->~WindowHandlerGLFW();
	_vkRenderer->~VkRenderer();
}

void RenderSystem::BeginFrame(bool& quit)
{
	_windowHandler->BeginFrame(quit);
	if (quit)
		return;

	_swapChain->BeginFrame();
}

void RenderSystem::EndFrame() const
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
