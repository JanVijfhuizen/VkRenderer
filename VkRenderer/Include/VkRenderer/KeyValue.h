#pragma once

namespace vi
{
	/// <summary>
	/// Data structure that can function as both a key and a value for containers like a map.
	/// </summary>
	template <typename Key, typename Value>
	struct KeyValue final
	{
		Key key;
		Value value;
	};
}