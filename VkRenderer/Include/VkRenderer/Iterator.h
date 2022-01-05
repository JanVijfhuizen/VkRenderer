#pragma once

namespace vi
{
	/// <summary>
	/// Standard iterator for data containers like vectors/arrays.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template <typename T>
	class Iterator final
	{
	public:
		T* begin;
		size_t length;
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

	template <typename T>
	T& Iterator<T>::operator*() const
	{
		return begin[index];
	}

	template <typename T>
	T& Iterator<T>::operator->() const
	{
		return begin[index];
	}

	template <typename T>
	const Iterator<T>& Iterator<T>::operator++()
	{
		++index;
		return *this;
	}

	template <typename T>
	Iterator<T> Iterator<T>::operator++(int)
	{
		Iterator temp(begin, length, index);
		++index;
		return temp;
	}
}
