#pragma once
#include "MapSet.h"

struct Camera final
{
	class System final : MapSet<Camera>, public Singleton<System>
	{
	public:
		System();
	};
};
