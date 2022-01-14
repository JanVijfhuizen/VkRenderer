#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"
#include "Vertex.h"

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
	struct VertexData final
	{
		vi::ArrayPtr<Vertex> vertices{};
		vi::ArrayPtr<Vertex::Index> indices{};
	};

	enum ForwardAxis
	{
		x, y, z
	};

	explicit MeshHandler(vi::VkCore& core);

	[[nodiscard]] static VertexData GenerateQuad(ForwardAxis axis = z, vi::FreeListAllocator& allocator = GMEM_TEMP);
	[[nodiscard]] static VertexData GenerateCube(vi::FreeListAllocator& allocator = GMEM_TEMP);

	[[nodiscard]] Mesh Create(const VertexData& vertexData) const;
	void Bind(Mesh& mesh);
	void Draw() const;
	void Destroy(const Mesh& mesh) const;

private:
	uint32_t _boundIndexCount = UINT32_MAX;
};
