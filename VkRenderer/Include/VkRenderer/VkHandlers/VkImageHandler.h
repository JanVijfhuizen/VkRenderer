#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkImageHandler final : public VkHandler
	{
		friend VkCore;

	public:

	private:
		explicit VkImageHandler(VkCore& core);
	};
}
