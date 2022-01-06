#pragma once

namespace vi
{
	class VkCore;

	class VkHandler
	{
	public:
		explicit VkHandler(VkCore& core);

	protected:
		VkCore& core;
	};
}