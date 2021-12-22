#pragma once
#include "UVector.h"
#include <cstdint>

template <typename Key, typename Value>
struct KeyValuePair final
{
	Key key;
	Value value;
};

template <typename Value>
class MapSet : public ce::Set, public UVector<KeyValuePair<uint32_t, Value>>
{
public:
	explicit MapSet(size_t size);

	Value& Insert(uint32_t index);
	KeyValuePair<uint32_t, Value>& Add(const KeyValuePair<uint32_t, Value>& keyPair) override;
	void Erase(uint32_t sparseId) override;
};

template <typename Value>
MapSet<Value>::MapSet(const size_t size) : UVector<KeyValuePair<uint32_t, Value>>(size)
{

}

template <typename Value>
Value& MapSet<Value>::Insert(const uint32_t index)
{
	return Add({ index, {} }).value;
}

template <typename Value>
KeyValuePair<uint32_t, Value>& MapSet<Value>::Add(const KeyValuePair<uint32_t, Value>& keyPair)
{
	const size_t count = UVector<KeyValuePair<uint32_t, Value>>::GetCount();
	for (size_t i = 0; i < count; ++i)
	{
		auto& value = UVector<KeyValuePair<uint32_t, Value>>::operator [](i);
		if (value.key == keyPair.key)
		{
			value = keyPair;
			return value;
		}
	}

	return UVector<KeyValuePair<uint32_t, Value>>::Add(keyPair);
}

template <typename Value>
void MapSet<Value>::Erase(const uint32_t sparseId)
{
	const size_t count = UVector<KeyValuePair<uint32_t, Value>>::GetCount() - 1;
	for (size_t i = 0; i < count; ++i)
	{
		auto& value = UVector<KeyValuePair<uint32_t, Value>>::operator [](i);
		if (value.key != sparseId)
			continue;

		UVector<KeyValuePair<uint32_t, Value>>::EraseAt(i);
		break;
	}
}
