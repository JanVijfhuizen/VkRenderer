#pragma once

namespace vi 
{
	class VkCore;
}

struct Material final
{
public:
	class System final : public ce::System<Material>
	{
	public:
		explicit System(ce::Cecsar& cecsar, vi::VkCore& core);

	private:
		vi::VkCore& _core;
	};
};
