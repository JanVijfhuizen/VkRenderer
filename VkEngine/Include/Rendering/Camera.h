#pragma once
#include "UVector.h"

struct Camera final
{
public:
	class System final : UVector<Camera>, public Singleton<System>
	{
	};
};
