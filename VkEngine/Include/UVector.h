#pragma once

template <typename T>
class UVector
{
public:
	class Iterator final
	{
	public:
		T* begin;
		size_t count;
		size_t index;

		T& operator*() const;
		T& operator->() const;

		const Iterator& operator++();
		Iterator operator++(int);

		friend auto operator==(const Iterator& a, const Iterator& b) -> bool
		{
			return a.index == b.index;
		};

		friend bool operator!= (const Iterator& a, const Iterator& b)
		{
			return !(a == b);
		}
	};

	[[nodiscard]] constexpr T& operator[](size_t sparseId);

	explicit UVector(size_t size = 8);
	virtual ~UVector();

	virtual T& Add(const T& value);
	virtual void EraseAt(size_t index);
	virtual void Clear();

	[[nodiscard]] size_t GetCount() const;
	[[nodiscard]] size_t GetSize() const;

	[[nodiscard]] constexpr Iterator begin();
	[[nodiscard]] Iterator end();
	
private:
	size_t _size;
	size_t _count = 0;
	T* _data = nullptr;
};

template <typename T>
constexpr T& UVector<T>::operator[](const size_t sparseId)
{
	return _data[sparseId];
}

template <typename T>
constexpr typename UVector<T>::Iterator UVector<T>::begin()
{
	Iterator it{};
	it.begin = _data;
	it.count = _count;
	it.index = 0;

	return it;
}

template <typename T>
typename UVector<T>::Iterator UVector<T>::end()
{
	Iterator it{};
	it.begin = _data;
	it.count = _count;
	it.index = _count;

	return it;
}

template <typename T>
T& UVector<T>::Iterator::operator*() const
{
	return begin[index];
}

template <typename T>
T& UVector<T>::Iterator::operator->() const
{
	return begin[index];
}

template <typename T>
const typename UVector<T>::Iterator& UVector<T>::Iterator::operator++()
{
	++index;
	return *this;
}

template <typename T>
typename UVector<T>::Iterator UVector<T>::Iterator::operator++(int)
{
	Iterator temp(begin, count, index);
	++index;
	return temp;
}

template <typename T>
UVector<T>::UVector(const size_t size) : _size(size)
{
	_data = reinterpret_cast<T*>(GMEM.MAlloc(sizeof(T) * size));
}

template <typename T>
UVector<T>::~UVector()
{
	GMEM.MFree(_data);
}

template <typename T>
T& UVector<T>::Add(const T& value)
{
	if(_count++ >= _size)
	{
		const size_t oldMemSize = sizeof(T) * _size;

		void* tempData = GMEM_TEMP.MAlloc(oldMemSize);
		memcpy(tempData, _data, oldMemSize);
		GMEM.Delete(_data);

		const size_t newCapacity = _size * 2;
		void* newData = GMEM.MAlloc(sizeof(T) * newCapacity);

		memcpy(newData, tempData, oldMemSize);
		GMEM_TEMP.Delete(tempData);

		_size = newCapacity;
		_data = reinterpret_cast<T*>(newData);
	}

	auto& t = _data[_count - 1];
	t = value;
	return t;
}

template <typename T>
void UVector<T>::EraseAt(const size_t index)
{
	_data[index] = _data[--_count];
}

template <typename T>
void UVector<T>::Clear()
{
	_count = 0;
}

template <typename T>
size_t UVector<T>::GetCount() const
{
	return _count;
}

template <typename T>
size_t UVector<T>::GetSize() const
{
	return _size;
}
