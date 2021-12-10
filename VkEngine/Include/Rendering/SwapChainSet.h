#pragma once

template <typename T>
class SwapChainSet final
{
public:
	struct Iterator final
	{
		explicit Iterator(T* data, size_t index);

		T& operator*() const;
		T& operator->() const;

		const Iterator& operator++();
		Iterator operator++(int);

		friend auto operator==(const Iterator& a, const Iterator& b) -> bool
		{
			return a._index == b._index;
		};

		friend bool operator!=(const Iterator& a, const Iterator& b)
		{
			return !(a == b);
		};

	private:
		size_t _index = 0;
		T* _data;
	};

	[[nodiscard]] constexpr T& operator[](size_t i);

	explicit SwapChainSet(size_t size);
	~SwapChainSet();

	[[nodiscard]] constexpr Iterator begin();
	[[nodiscard]] constexpr Iterator end();

private:
	T* _data;
	size_t _size;
};

template <typename T>
constexpr T& SwapChainSet<T>::operator[](const size_t i)
{
	return _data[i];
}

template <typename T>
constexpr typename SwapChainSet<T>::Iterator SwapChainSet<T>::begin()
{
	return Iterator{ _data, 0 };
}

template <typename T>
constexpr typename SwapChainSet<T>::Iterator SwapChainSet<T>::end()
{
	return Iterator{ _data, _size };
}

template <typename T>
SwapChainSet<T>::Iterator::Iterator(T* data, const size_t index) : 
	_data(data), _index(index)
{
}

template <typename T>
T& SwapChainSet<T>::Iterator::operator*() const
{
	return _data[_index];
}

template <typename T>
T& SwapChainSet<T>::Iterator::operator->() const
{
	return _data[_index];
}

template <typename T>
const typename SwapChainSet<T>::Iterator& SwapChainSet<T>::Iterator::operator++()
{
	++_index;
	return *this;
}

template <typename T>
typename SwapChainSet<T>::Iterator SwapChainSet<T>::Iterator::operator++(int)
{
	Iterator temp{ *this };
	++temp._index;
	return temp;
}

template <typename T>
SwapChainSet<T>::SwapChainSet(const size_t size) : _size(size)
{
	_data = reinterpret_cast<T*>(GMEM.MAlloc(sizeof(T) * size));

	for (size_t i = 0; i < size; ++i)
		_data[i] = {};
}

template <typename T>
SwapChainSet<T>::~SwapChainSet()
{
	GMEM.Free(_data);
}
