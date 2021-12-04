#include "pch.h"
#include "Rendering/Mesh.h"
#include "VkRenderer/VkRenderer.h"
#include "Rendering/RenderSystem.h"

Mesh::System::System(const uint32_t capacity) : SparseSet<Mesh>(capacity)
{

}

uint32_t Mesh::System::Create(const Vertex::Data& vertexData)
{
	auto& renderSystem = Renderer::System::Get();
	auto& vkRenderer = renderSystem.GetVkRenderer();

	auto& vertices = vertexData.vertices;
	auto& indices = vertexData.indices;

	auto cpyCommandBuffer = vkRenderer.CreateCommandBuffer();
	const auto cpyFence = vkRenderer.CreateFence();

	// Send vertex data.
	const auto vertStagingBuffer = vkRenderer.CreateBuffer(sizeof(Vertex) * vertices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto vertStagingMem = vkRenderer.AllocateMemory(vertStagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vkRenderer.BindMemory(vertStagingBuffer, vertStagingMem);
	vkRenderer.MapMemory(vertStagingMem, vertices.data(), 0, sizeof(Vertex) * vertices.size());

	const auto vertBuffer = vkRenderer.CreateBuffer(sizeof(Vertex) * vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	const auto vertMem = vkRenderer.AllocateMemory(vertBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkRenderer.BindMemory(vertBuffer, vertMem);

	// Send indices data.
	const auto indStagingBuffer = vkRenderer.CreateBuffer(sizeof(uint16_t) * indices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto indStagingMem = vkRenderer.AllocateMemory(indStagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vkRenderer.BindMemory(indStagingBuffer, indStagingMem);
	vkRenderer.MapMemory(indStagingMem, indices.data(), 0, sizeof(uint16_t) * indices.size());

	const auto indBuffer = vkRenderer.CreateBuffer(sizeof(Vertex) * vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	const auto indMem = vkRenderer.AllocateMemory(indBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkRenderer.BindMemory(indBuffer, indMem);

	vkRenderer.BeginCommandBufferRecording(cpyCommandBuffer);
	vkRenderer.CopyBuffer(vertStagingBuffer, vertBuffer, vertices.size() * sizeof(Vertex));
	vkRenderer.CopyBuffer(indStagingBuffer, indBuffer, indices.size() * sizeof(uint16_t));
	vkRenderer.EndCommandBufferRecording();

	vkRenderer.Submit(&cpyCommandBuffer, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, cpyFence);
	vkRenderer.WaitForFence(cpyFence);

	vkRenderer.DestroyBuffer(vertStagingBuffer);
	vkRenderer.FreeMemory(vertStagingMem);

	vkRenderer.DestroyBuffer(indStagingBuffer);
	vkRenderer.FreeMemory(indStagingMem);

	vkRenderer.DestroyFence(cpyFence);
	vkRenderer.DestroyCommandBuffer(cpyCommandBuffer);

	Data data{};
	data.vertexBuffer = vertBuffer;
	data.vertexMemory = vertMem;
	data.indexBuffer = indBuffer;
	data.indexMemory = indMem;
	data.indCount = indices.size();

	return -1;
}
