#pragma once
#include "RenderSet.h"

struct Material final
{
	struct Frame final
	{
		
	};

	class System final : public RenderSet<Material, Frame>, public Singleton<System>
	{
	public:
		explicit System(uint32_t size);
	};
};
