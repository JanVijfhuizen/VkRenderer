#pragma once

struct Transform final
{
	glm::vec3 position{};
	glm::vec3 rotation{};
	glm::vec3 scale{1};

	class System final : public ce::SparseSet<Transform>, public Singleton<System>
	{
	public:
		explicit System(uint32_t capacity);
	};
};
