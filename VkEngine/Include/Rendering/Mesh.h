#pragma once
#include "Vertex.h"

struct Mesh final
{
	uint32_t handle = 0;

	class System final : public ce::SparseSet<Mesh>, public Singleton<System>
	{
	public:
		explicit System(uint32_t capacity);

		[[nodiscard]] uint32_t Create(const Vertex::Data& vertexData);

	private:
		struct Data final
		{
			VkBuffer vertexBuffer;
			VkDeviceMemory vertexMemory;
			VkBuffer indexBuffer;
			VkDeviceMemory indexMemory;
			uint32_t indCount;
		};
	};
};
