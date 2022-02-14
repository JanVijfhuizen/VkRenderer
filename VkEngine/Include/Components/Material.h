#pragma once
#include "Rendering/MeshHandler.h"
#include "Rendering/TextureHandler.h"

struct Mesh;
struct Texture;
class VulkanRenderer;

/// <summary>
/// Component that handles the standard rendering for an entity.
/// </summary>
struct Material
{
	Mesh* mesh = nullptr;
	Texture* texture = nullptr;
};

/// <summary>
/// System that handles the material components.
/// </summary>
class MaterialSystem final : public ce::System<Material>
{
public:
	explicit MaterialSystem(ce::Cecsar& cecsar, VulkanRenderer& renderer);
	~MaterialSystem();

	[[nodiscard]] Mesh& GetFallbackMesh();
	[[nodiscard]] Texture& GetFallbackTexture();

private:
	VulkanRenderer& _renderer;
	Mesh _fallbackMesh;
	Texture _fallbackTexture;
};
