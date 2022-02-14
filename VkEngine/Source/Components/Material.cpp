#include "pch.h"
#include "Components/Material.h"
#include "Rendering/MeshHandler.h"
#include "Rendering/VulkanRenderer.h"
#include "Rendering/TextureHandler.h"

MaterialSystem::MaterialSystem(ce::Cecsar& cecsar, VulkanRenderer& renderer) : 
	System<Material>(cecsar), _renderer(renderer)
{
	auto& meshHandler = renderer.GetMeshHandler();
	auto& textureHandler = renderer.GetTextureHandler();

	// Create fallback mesh and texture.
	_fallbackMesh = meshHandler.Create(MeshHandler::GenerateCube());
	_fallbackTexture = textureHandler.Create("Test", "png");
}

MaterialSystem::~MaterialSystem()
{
	auto& meshHandler = _renderer.GetMeshHandler();
	auto& textureHandler = _renderer.GetTextureHandler();

	textureHandler.Destroy(_fallbackTexture);
	meshHandler.Destroy(_fallbackMesh);
}

Mesh& MaterialSystem::GetFallbackMesh()
{
	return _fallbackMesh;
}

Texture& MaterialSystem::GetFallbackTexture()
{
	return _fallbackTexture;
}
