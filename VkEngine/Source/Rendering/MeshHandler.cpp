#include "pch.h"
#include "Rendering/MeshHandler.h"

MeshHandler::MeshHandler(vi::VkCore& core): VkHandler(core)
{
}

MeshHandler::VertexData MeshHandler::GenerateQuad(vi::FreeListAllocator& allocator)
{
	VertexData vertexData{};

	vertexData.vertices = vi::ArrayPtr<Vertex>(4, allocator);
	auto& lBot = vertexData.vertices[0];
	lBot.position = { -1, -1, 0 };
	lBot.textureCoordinates = { 0, 0 };
	auto& lTop = vertexData.vertices[1];
	lTop.position = { -1, 1, 0 };
	lTop.textureCoordinates = { 0, 1 };
	auto& rTop = vertexData.vertices[2];
	rTop.position = { 1, 1, 0 };
	rTop.textureCoordinates = { 1, 1 };
	auto& rBot = vertexData.vertices[3];
	rBot.position = { 1, -1, 0 };
	rBot.textureCoordinates = { 1, 0 };

	Vertex::Index indices[] = { 0, 1, 2, 0, 2, 3 };
	vertexData.indices = vi::ArrayPtr<Vertex::Index>{ indices, 6, allocator };

	return vertexData;
}

MeshHandler::Mesh MeshHandler::Create(const VertexData& vertexData)
{
	return {};
}

void MeshHandler::Destroy(const Mesh& mesh)
{
}
