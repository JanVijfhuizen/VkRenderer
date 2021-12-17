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

	void Add(const KeyValuePair<uint32_t, Value>& keyPair) override;
	void Erase(uint32_t sparseId) override;
};

template <typename Value>
MapSet<Value>::MapSet(const size_t size) : UVector<KeyValuePair<uint32_t, Value>>(size)
{

}

template <typename Value>
void MapSet<Value>::Add(const KeyValuePair<uint32_t, Value>& keyPair)
{
	for (size_t i = UVector<KeyValuePair<uint32_t, Value>>::GetCount() - 1; i >= 0; --i)
	{
		auto& value = UVector<KeyValuePair<uint32_t, Value>>::operator [](i);
		if (value.key == keyPair.key)
		{
			value = keyPair;
			return;
		}
	}

	UVector<KeyValuePair<uint32_t, Value>>::Add(keyPair);
}

template <typename Value>
void MapSet<Value>::Erase(const uint32_t sparseId)
{
	for (size_t i = UVector<KeyValuePair<uint32_t, Value>>::GetCount() - 1; i >= 0; --i)
	{
		auto& value = UVector<KeyValuePair<uint32_t, Value>>::operator [](i);
		if (value.key != sparseId)
			continue;

		UVector<KeyValuePair<uint32_t, Value>>::EraseAt(i);
		break;
	}
}
