#pragma once

struct DefaultMaterial final
{
public:
	class System final : public ce::SparseSet<DefaultMaterial>, public Singleton<System>
	{
	public:
		explicit System(uint32_t size);
		~System();
	};
};
