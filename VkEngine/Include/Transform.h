#pragma once

struct Transform final
{
	class System final : public ce::SparseSet<Transform>, public Singleton<System>
	{
	public:
		explicit System(uint32_t capacity);
	};
};
