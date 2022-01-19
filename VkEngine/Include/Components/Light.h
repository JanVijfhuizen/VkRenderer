#pragma once
#include "Rendering/SwapChainExt.h"

struct Light final
{
	
};

struct ShadowCaster final
{
	
};

class LightSystem final : public ce::SmallSystem<Light>, SwapChainExt::Dependency
{
public:
	LightSystem(ce::Cecsar& cecsar, Renderer& renderer, size_t size);

protected:
	void OnRecreateSwapChainAssets() override;
};

class ShadowCasterSystem final : public ce::System<ShadowCaster>
{
public:
	explicit ShadowCasterSystem(ce::Cecsar& cecsar);
};
