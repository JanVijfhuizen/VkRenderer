#pragma once

struct Transform final
{
	glm::vec3 position{};
	glm::vec3 rotation{};
	glm::vec3 scale{1};
	bool manualBake = false;

	struct Baked final
	{
		glm::mat4 model{1};
	};

	class System final : public ce::SparseSet<Transform>, public Singleton<System>
	{
	public:
		explicit System(uint32_t size);
		~System();

		void Update();
		void Swap(uint32_t aDenseId, uint32_t bDenseId) override;

		[[nodiscard]] Baked* GetBakedTransforms() const;

	private:
		Baked* _bakedArr;
	};
};
