#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"
#include "Vertex.h"
#include "VkRenderer/VkCore/VkCore.h"
#include "VkRenderer/VkHandlers/VkCommandBufferHandler.h"
#include "VkRenderer/VkHandlers/VkShaderHandler.h"
#include "VkRenderer/VkHandlers/VkMemoryHandler.h"
#include "VkRenderer/VkHandlers/VkSyncHandler.h"

struct Mesh final
{
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexMemory;
	uint32_t indexCount;
};

class MeshHandler final : public vi::VkHandler
{
public:
	template <typename Vert = Vertex, typename Ind = Vertex::Index>
	struct VertexData final
	{
		vi::ArrayPtr<Vert> vertices{};
		vi::ArrayPtr<Ind> indices{};
	};

	enum ForwardAxis
	{
		x, y, z
	};

	explicit MeshHandler(vi::VkCore& core);

	[[nodiscard]] static VertexData<Vertex, Vertex::Index> GenerateQuad(ForwardAxis axis = z, bool counterClockwise = false, vi::FreeListAllocator& allocator = GMEM_TEMP);
	[[nodiscard]] static VertexData<Vertex, Vertex::Index> GenerateCube(vi::FreeListAllocator& allocator = GMEM_TEMP);

	template <typename Vert = Vertex, typename Ind = Vertex::Index>
	[[nodiscard]] Mesh Create(const VertexData<Vert, Ind>& vertexData) const;
	void Bind(Mesh& mesh);
	void Draw() const;
	void Destroy(const Mesh& mesh) const;

private:
	uint32_t _boundIndexCount = UINT32_MAX;
};

template <typename Vert, typename Ind>
Mesh MeshHandler::Create(const VertexData<Vert, Ind>& vertexData) const
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
	const auto vertStagingBuffer = shaderHandler.CreateBuffer(sizeof(Vert) * vertices.GetLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto vertStagingMem = memoryHandler.Allocate(vertStagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	memoryHandler.Bind(vertStagingBuffer, vertStagingMem);
	memoryHandler.Map(vertStagingMem, vertices.GetData(), 0, sizeof(Vert) * vertices.GetLength());

	// Allocate shader efficient memory for the vertices.
	const auto vertBuffer = shaderHandler.CreateBuffer(sizeof(Vert) * vertices.GetLength(),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	const auto vertMem = memoryHandler.Allocate(vertBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	memoryHandler.Bind(vertBuffer, vertMem);

	// Allocate staging memory for indices.
	const auto indStagingBuffer = shaderHandler.CreateBuffer(sizeof(Ind) * indices.GetLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto indStagingMem = memoryHandler.Allocate(indStagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	memoryHandler.Bind(indStagingBuffer, indStagingMem);
	memoryHandler.Map(indStagingMem, indices.GetData(), 0, sizeof(Ind) * indices.GetLength());

	// Allocate shader efficient memory for the indices.
	const auto indBuffer = shaderHandler.CreateBuffer(sizeof(Ind) * indices.GetLength(),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	const auto indMem = memoryHandler.Allocate(indBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	memoryHandler.Bind(indBuffer, indMem);

	// Copy data from the staging buffers to the gpu efficient buffers.
	commandBufferHandler.BeginRecording(cpyCommandBuffer);
	shaderHandler.CopyBuffer(vertStagingBuffer, vertBuffer, vertices.GetLength() * sizeof(Vert));
	shaderHandler.CopyBuffer(indStagingBuffer, indBuffer, indices.GetLength() * sizeof(Ind));
	commandBufferHandler.EndRecording();

	vi::VkCommandBufferHandler::SubmitInfo submitInfo{};
	submitInfo.buffers = &cpyCommandBuffer;
	submitInfo.buffersCount = 1;
	submitInfo.fence = cpyFence;
	commandBufferHandler.Submit(submitInfo);
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
