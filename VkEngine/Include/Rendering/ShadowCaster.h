#pragma once

struct ShadowCaster final
{
public:
	class System final : public ce::SparseSet<ShadowCaster>, public Singleton<System>
	{
	public:
		explicit System(uint32_t size);
	};
};
