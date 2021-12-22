#pragma once
#include "Vertex.h"
#include "UVector.h"

struct Mesh final
{
	uint32_t handle = 0;

	struct Data final
	{
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexMemory;
		uint32_t indexCount;
	};

	class System final : public ce::SparseSet<Mesh>, public Singleton<System>
	{
	public:
		explicit System(uint32_t size);
		~System();

		[[nodiscard]] uint32_t Create(const Vertex::Data& vertData);
		[[nodiscard]] const Data& GetData(Mesh& mesh);

	private:
		UVector<Data> _data{};
	};
};
