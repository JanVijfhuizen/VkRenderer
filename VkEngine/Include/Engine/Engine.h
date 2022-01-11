#pragma once
#include "VkRenderer/WindowHandlerGLFW.h"
#include "Components/Material.h"
#include "Components/Transform.h"
#include "Rendering/Renderer.h"
#include "Components/Camera.h"

class Engine
{
public:
	struct Info final
	{
		size_t capacity = 1e2f;

		typedef void (*Awake)();
		typedef void (*Start)();
		typedef void (*Update)(bool& outQuit);
		typedef void (*Cleanup)();

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

protected:
	[[nodiscard]] Renderer& GetRenderer() const;
	[[nodiscard]] ce::Cecsar& GetCecsar() const;
	[[nodiscard]] CameraSystem& GetCameras() const;
	[[nodiscard]] TransformSystem& GetTransforms() const;
	[[nodiscard]] MaterialSystem& GetMaterials() const;

private:
	bool _isRunning = false;
	vi::UniquePtr<vi::WindowHandlerGLFW> _windowHandler;
	vi::UniquePtr<Renderer> _renderer;
	vi::UniquePtr<ce::Cecsar> _cecsar;
	vi::UniquePtr<TransformSystem> _transforms;
	vi::UniquePtr<CameraSystem> _cameras;
	vi::UniquePtr<MaterialSystem> _materials;
};
