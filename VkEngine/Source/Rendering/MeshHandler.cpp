#include "pch.h"
#include "Rendering/MeshHandler.h"
#include "VkRenderer/VkCore/VkCore.h"

MeshHandler::MeshHandler(vi::VkCore& core): VkHandler(core)
{
}

MeshHandler::VertexData MeshHandler::GenerateQuad(const ForwardAxis axis, vi::FreeListAllocator& allocator)
{
	VertexData vertexData{};

	vertexData.vertices = vi::ArrayPtr<Vertex>(4, allocator);
	auto& lBot = vertexData.vertices[0];
	lBot.position = { -1, -1 * (axis == z), -1 * (axis == y) };
	lBot.textureCoordinates = { 0, 0 };
	auto& lTop = vertexData.vertices[1];
	lTop.position = { -1, 1 * (axis == z), 1 * (axis == y) };
	lTop.textureCoordinates = { 0, 1 };
	auto& rTop = vertexData.vertices[2];
	rTop.position = { 1, 1 * (axis == z), 1 * (axis == y) };
	rTop.textureCoordinates = { 1, 1 };
	auto& rBot = vertexData.vertices[3];
	rBot.position = { 1, -1 * (axis == z), -1 * (axis == y) };
	rBot.textureCoordinates = { 1, 0 };

	const glm::vec3 forward = glm::vec3(axis == x, axis == y, axis == z);
	for (auto& vertex : vertexData.vertices)
		vertex.normal = forward;

	Vertex::Index indices[] = { 0, 1, 2, 0, 2, 3 };
	vertexData.indices = vi::ArrayPtr<Vertex::Index>{ indices, 6, allocator };

	return vertexData;
}

MeshHandler::VertexData MeshHandler::GenerateCube(vi::FreeListAllocator& allocator)
{
	VertexData quads[4];

	auto& top = quads[0] = GenerateQuad(y, allocator);
	for (auto& vertex : top.vertices)
		vertex.position.y = 1;

	auto& bot = quads[1] = GenerateQuad(y, allocator);
	for (auto& vertex : bot.vertices)
	{
		vertex.position.y = -1;
		vertex.normal.y *= -1;
	}

	auto& front = quads[2] = GenerateQuad(z, allocator);
	for (auto& vertex : front.vertices)
		vertex.position.z = 1;

	auto& back = quads[3] = GenerateQuad(z, allocator);
	for (auto& vertex : back.vertices)
	{
		vertex.position.z = -1;
		vertex.normal.z *= -1;
	}

	VertexData cubeData{};
	const size_t size = sizeof quads / sizeof(VertexData);
	const size_t vertLength = quads[0].vertices.GetLength();
	const size_t indLength = quads[0].indices.GetLength();
	cubeData.vertices.Reallocate(sizeof(Vertex) * vertLength * size, allocator);
	cubeData.indices.Reallocate(sizeof(Vertex::Index) * indLength * size, allocator);

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

Mesh MeshHandler::Create(const VertexData& vertexData) const
{
	auto& vertices = vertexData.vertices;
	auto& indices = vertexData.indices;

	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& memoryHandler = core.GetMemoryHandler();
	auto& shaderHandler = core.GetShaderHandler();
	auto& syncHandler = core.GetSyncHandler();

	auto cpyCommandBuffer = commandBufferHandler.Create();
	const auto cpyFence = syncHandler.CreateFence();

	// Allocate staging memory for vertices.
	const auto vertStagingBuffer = shaderHandler.CreateBuffer(sizeof(Vertex) * vertices.GetLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto vertStagingMem = memoryHandler.Allocate(vertStagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	memoryHandler.Bind(vertStagingBuffer, vertStagingMem);
	memoryHandler.Map(vertStagingMem, vertices.GetData(), 0, sizeof(Vertex) * vertices.GetLength());

	// Allocate shader efficient memory for the vertices.
	const auto vertBuffer = shaderHandler.CreateBuffer(sizeof(Vertex) * vertices.GetLength(), 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	const auto vertMem = memoryHandler.Allocate(vertBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	memoryHandler.Bind(vertBuffer, vertMem);

	// Allocate staging memory for indices.
	const auto indStagingBuffer = shaderHandler.CreateBuffer(sizeof(Vertex::Index) * indices.GetLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto indStagingMem = memoryHandler.Allocate(indStagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	memoryHandler.Bind(indStagingBuffer, indStagingMem);
	memoryHandler.Map(indStagingMem, indices.GetData(), 0, sizeof(Vertex::Index) * indices.GetLength());

	// Allocate shader efficient memory for the indices.
	const auto indBuffer = shaderHandler.CreateBuffer(sizeof(Vertex) * vertices.GetLength(), 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	const auto indMem = memoryHandler.Allocate(indBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	memoryHandler.Bind(indBuffer, indMem);

	// Copy data from the staging buffers to the gpu efficient buffers.
	commandBufferHandler.BeginRecording(cpyCommandBuffer);
	shaderHandler.CopyBuffer(vertStagingBuffer, vertBuffer, vertices.GetLength() * sizeof(Vertex));
	shaderHandler.CopyBuffer(indStagingBuffer, indBuffer, indices.GetLength() * sizeof(Vertex::Index));
	commandBufferHandler.EndRecording();

	commandBufferHandler.Submit(&cpyCommandBuffer, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, cpyFence);
	syncHandler.WaitForFence(cpyFence);

	// Free staging buffers/memory.
	shaderHandler.DestroyBuffer(vertStagingBuffer);
	memoryHandler.Free(vertStagingMem);
	shaderHandler.DestroyBuffer(indStagingBuffer);
	memoryHandler.Free(indStagingMem);

	syncHandler.DestroyFence(cpyFence);
	commandBufferHandler.Destroy(cpyCommandBuffer);

	Mesh mesh{};
	mesh.vertexBuffer = vertBuffer;
	mesh.vertexMemory = vertMem;
	mesh.indexBuffer = indBuffer;
	mesh.indexMemory = indMem;
	mesh.indexCount = indices.GetLength();
	return mesh;
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
