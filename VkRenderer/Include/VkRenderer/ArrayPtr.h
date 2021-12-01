#pragma once
#include <cstdint>

template <typename T>
class ArrayPtr final
{
public:
	class Iterator final
	{
	public:
		T* begin;
		size_t size;
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

	[[nodiscard]] constexpr T& operator[](uint32_t index);

	explicit ArrayPtr(T* begin, size_t size);

	[[nodiscard]] constexpr Iterator begin();
	[[nodiscard]] Iterator end();

private:
	T* _begin;
	size_t _size;
};

template <typename T>
constexpr T& ArrayPtr<T>::operator[](const uint32_t index)
{
	return _begin[index];
}

template <typename T>
constexpr typename ArrayPtr<T>::Iterator ArrayPtr<T>::begin()
{
	Iterator it;
	it.begin = _begin;
	it.size = _size;
	it.index = 0;

	return it;
}

template <typename T>
typename ArrayPtr<T>::Iterator ArrayPtr<T>::end()
{
	Iterator it;
	it.begin = _begin;
	it.size = _size;
	it.index = _size;

	return it;
}

template <typename T>
T& ArrayPtr<T>::Iterator::operator*() const
{
	return begin[index];
}

template <typename T>
T& ArrayPtr<T>::Iterator::operator->() const
{
	return begin[index];
}

template <typename T>
const typename ArrayPtr<T>::Iterator& ArrayPtr<T>::Iterator::operator++()
{
	++index;
	return *this;
}

template <typename T>
typename ArrayPtr<T>::Iterator ArrayPtr<T>::Iterator::operator++(int)
{
	Iterator temp(begin, size, index);
	++index;
	return temp;
}

template <typename T>
ArrayPtr<T>::ArrayPtr(T* begin, const size_t size) : _begin(begin), _size(size)
{

}