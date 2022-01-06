#pragma once

namespace vi
{
	class VkCore;

	/// <summary>
	/// Base class for specialized VkCore handlers to inherit from.
	/// </summary>
	class VkHandler
	{
	public:
		explicit VkHandler(VkCore& core);

	protected:
		VkCore& core;
	};
}