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

		friend bool operator==(const Iterator& a, const Iterator& b)
		{
			return a.index == b.index;
		};

		friend bool operator!= (const Iterator& a, const Iterator& b)
		{
			return !(a == b);
		}
	};

	[[nodiscard]] constexpr T& operator[](uint32_t index) const;

	ArrayPtr();
	explicit ArrayPtr(void* begin, size_t size);

	[[nodiscard]] constexpr Iterator begin() const;
	[[nodiscard]] Iterator end() const;

	[[nodiscard]] constexpr bool IsNull() const;
	[[nodiscard]] constexpr size_t GetSize() const;
	[[nodiscard]] constexpr T* GetData() const;

private:
	T* _data = nullptr;
	size_t _size = 0;
};

template <typename T>
constexpr T& ArrayPtr<T>::operator[](const uint32_t index) const
{
	return _data[index];
}

template <typename T>
constexpr typename ArrayPtr<T>::Iterator ArrayPtr<T>::begin() const
{
	Iterator it{};
	it.begin = _data;
	it.size = _size;
	it.index = 0;

	return it;
}

template <typename T>
constexpr bool ArrayPtr<T>::IsNull() const
{
	return GetSize() == 0;
}

template <typename T>
constexpr size_t ArrayPtr<T>::GetSize() const
{
	return _size;
}

template <typename T>
constexpr T* ArrayPtr<T>::GetData() const
{
	return _data;
}

template <typename T>
typename ArrayPtr<T>::Iterator ArrayPtr<T>::end() const
{
	Iterator it{};
	it.begin = _data;
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
ArrayPtr<T>::ArrayPtr() = default;

template <typename T>
ArrayPtr<T>::ArrayPtr(void* begin, const size_t size) : _data(reinterpret_cast<T*>(begin)), _size(size)
{

}
