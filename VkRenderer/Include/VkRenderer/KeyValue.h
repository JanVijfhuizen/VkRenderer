#pragma once

namespace vi
{
	template <typename Key, typename Value>
	struct KeyValue final
	{
		Key key;
		Value value;
	};
}