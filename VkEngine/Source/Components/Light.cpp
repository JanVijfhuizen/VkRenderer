#include "pch.h"
#include "Components/Light.h"

LightSystem::LightSystem(ce::Cecsar& cecsar, Renderer& renderer, const size_t size) : 
	SmallSystem<Light>(cecsar, size), Dependency(renderer)
{
}

void LightSystem::OnRecreateSwapChainAssets()
{

}

ShadowCasterSystem::ShadowCasterSystem(ce::Cecsar& cecsar) : System<ShadowCaster>(cecsar)
{
}
