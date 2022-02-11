#include "pch.h"
#include "Rendering/MeshHandler.h"
#include "VkRenderer/VkCore/VkCore.h"

MeshHandler::MeshHandler(vi::VkCore& core): VkHandler(core)
{
}

MeshHandler::VertexData<Vertex, Vertex::Index> MeshHandler::GenerateQuad(const ForwardAxis axis,
	const bool counterClockwise, vi::FreeListAllocator& allocator)
{
	VertexData<Vertex, Vertex::Index> vertexData{};

	const float xMul = static_cast<float>(axis == x) * -2.f + 1.f;

	vertexData.vertices = vi::ArrayPtr<Vertex>(4, allocator);
	auto& lBot = vertexData.vertices[0];
	lBot.position = { -1 * (axis != x), -1 * (axis != y), -1 * (axis != z) * xMul };
	lBot.textureCoordinates = { 0, 0 };
	auto& lTop = vertexData.vertices[1];
	lTop.position = { -1 * (axis != x), 1 * (axis != y), 1 * (axis != z) };
	lTop.textureCoordinates = { 0, 1 };
	auto& rTop = vertexData.vertices[2];
	rTop.position = { 1 * (axis != x), 1 * (axis != y), 1 * (axis != z) * xMul };
	rTop.textureCoordinates = { 1, 1 };
	auto& rBot = vertexData.vertices[3];
	rBot.position = { 1 * (axis != x), -1 * (axis != y), -1 * (axis != z) };
	rBot.textureCoordinates = { 1, 0 };

	const float dir = 2.f * static_cast<float>(counterClockwise) - 1.f;
	const glm::vec3 forward = glm::vec3(axis == x, axis == y, axis == z) * dir;
	for (auto& vertex : vertexData.vertices)
		vertex.normal = forward;

	Vertex::Index indices[6] = { 0, 1, 2, 0, 2, 3 };
	if(counterClockwise)
	{
		auto a = indices[1];
		indices[1] = indices[2];
		indices[2] = a;

		auto b = indices[4];
		indices[4] = indices[5];
		indices[5] = b;
	}

	vertexData.indices = vi::ArrayPtr<Vertex::Index>{ indices, 6, allocator };

	return vertexData;
}

MeshHandler::VertexData<Vertex, Vertex::Index> MeshHandler::GenerateCube(vi::FreeListAllocator& allocator)
{
	VertexData<Vertex, Vertex::Index> quads[6];

	auto& top = quads[0] = GenerateQuad(y, false, allocator);
	for (auto& vertex : top.vertices)
		vertex.position.y = 1;

	auto& bot = quads[1] = GenerateQuad(y, true, allocator);
	for (auto& vertex : bot.vertices)
		vertex.position.y = -1;

	auto& front = quads[2] = GenerateQuad(z, true, allocator);
	for (auto& vertex : front.vertices)
		vertex.position.z = 1;

	auto& back = quads[3] = GenerateQuad(z, false, allocator);
	for (auto& vertex : back.vertices)
		vertex.position.z = -1;

	auto& right = quads[4] = GenerateQuad(x, true, allocator);
	for (auto& vertex : right.vertices)
		vertex.position.x = 1;

	auto& left = quads[5] = GenerateQuad(x, false, allocator);
	for (auto& vertex : left.vertices)
		vertex.position.x = -1;

	VertexData<Vertex, Vertex::Index> cubeData{};
	const size_t size = sizeof quads / sizeof(VertexData<Vertex, Vertex::Index>);
	const size_t vertLength = quads[0].vertices.GetLength();
	const size_t indLength = quads[0].indices.GetLength();
	cubeData.vertices.Reallocate(vertLength * size, allocator);
	cubeData.indices.Reallocate(indLength * size, allocator);

	size_t vertOffset = 0;
	size_t indOffset = 0;

	for (auto& quad : quads)
	{
		for (auto& index : quad.indices)
			index += vertOffset;
		
		cubeData.vertices.CopyData(quad.vertices, vertOffset, vertOffset + vertLength);
		cubeData.indices.CopyData(quad.indices, indOffset, indOffset + indLength);

		vertOffset += vertLength;
		indOffset += indLength;
	}

	return cubeData;
}

void MeshHandler::Bind(Mesh& mesh)
{
	auto& shaderHandler = core.GetShaderHandler();
	shaderHandler.BindVertexBuffer(mesh.vertexBuffer);
	shaderHandler.BindIndicesBuffer(mesh.indexBuffer);
	_boundIndexCount = mesh.indexCount;
}

void MeshHandler::Draw() const
{
	assert(_boundIndexCount != UINT32_MAX);
	core.GetShaderHandler().Draw(_boundIndexCount);
}

void MeshHandler::Destroy(const Mesh& mesh) const
{
	auto& memoryHandler = core.GetMemoryHandler();
	auto& shaderHandler = core.GetShaderHandler();

	shaderHandler.DestroyBuffer(mesh.vertexBuffer);
	memoryHandler.Free(mesh.vertexMemory);
	shaderHandler.DestroyBuffer(mesh.indexBuffer);
	memoryHandler.Free(mesh.indexMemory);
}
