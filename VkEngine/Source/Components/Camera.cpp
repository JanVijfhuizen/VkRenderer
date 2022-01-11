#include "pch.h"
#include "Components/Camera.h"

CameraSystem::CameraSystem(ce::Cecsar& cecsar) : SmallSystem<Camera>(cecsar, MAX_CAMERAS)
{
}
