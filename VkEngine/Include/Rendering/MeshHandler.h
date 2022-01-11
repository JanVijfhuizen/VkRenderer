#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"
#include "Vertex.h"

class MeshHandler final : public vi::VkHandler
{
public:
	struct VertexData final
	{
		vi::ArrayPtr<Vertex> vertices{};
		vi::ArrayPtr<Vertex::Index> indices{};
	};

	struct Mesh final
	{
		
	};

	explicit MeshHandler(vi::VkCore& core);

	[[nodiscard]] static VertexData GenerateQuad(vi::FreeListAllocator& allocator = GMEM_TEMP);

	[[nodiscard]] Mesh Create(const VertexData& vertexData);
	void Destroy(const Mesh& mesh);
};
