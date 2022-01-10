#pragma once

namespace vi
{
	/// <summary>
	/// A resizeable array.
	/// </summary>
	template <typename T>
	class Vector final : public ArrayPtr<T>
	{
	public:
		Vector();
		explicit Vector(size_t size, FreeListAllocator& allocator, size_t count = 0);

		/// <summary>
		/// Adjusts the count value. Resizes if count is larger than the vector's length.
		/// </summary>
		/// <param name="count">New count.</param>
		void Resize(size_t count);
		T& Add(const T& value = {});
		void RemoveAt(size_t index);
		/// <summary>Removes all elements and sets count to zero.</summary>
		void Clear();

		/// <summary>Removes the first instance in the vector and returns it. </summary>
		T Pop();

		[[nodiscard]] size_t GetCount() const;

		[[nodiscard]] Iterator<T> end() const override;

	private:
		size_t _count = 0;
	};

	template <typename T>
	Vector<T>::Vector() = default;

	template <typename T>
	Vector<T>::Vector(const size_t size, FreeListAllocator& allocator, const size_t count) : ArrayPtr<T>(size, allocator), _count(count)
	{
		assert(_count <= size);
	}

	template <typename T>
	void Vector<T>::Resize(size_t count)
	{
		FreeListAllocator* allocator = ArrayPtr<T>::GetAllocator();
		assert(allocator);

		const size_t length = ArrayPtr<T>::GetLength();
		if (length < count)
			ArrayPtr<T>::Reallocate(count, *allocator);
		_count = count;
	}

	template <typename T>
	T& Vector<T>::Add(const T& value)
	{
		auto allocator = ArrayPtr<T>::GetAllocator();
		assert(allocator);
		const size_t length = ArrayPtr<T>::GetLength();
		if(++_count >= length)
			ArrayPtr<T>::Reallocate(Ut::Max<size_t>(1, length * 2), *allocator);
		return ArrayPtr<T>::operator[](_count - 1) = value;
	}

	template <typename T>
	void Vector<T>::RemoveAt(const size_t index)
	{
		ArrayPtr<T>::operator[](index) = ArrayPtr<T>::operator[](_count-- -1);
	}

	template <typename T>
	void Vector<T>::Clear()
	{
		_count = 0;
	}

	template <typename T>
	T Vector<T>::Pop()
	{
		const T instance = ArrayPtr<T>::GetData()[0];
		RemoveAt(0);
		return instance;
	}

	template <typename T>
	size_t Vector<T>::GetCount() const
	{
		return _count;
	}

	template <typename T>
	Iterator<T> Vector<T>::end() const
	{
		Iterator<T> it{};
		it.begin = ArrayPtr<T>::GetData();
		it.length = _count;
		it.index = _count;

		return it;
	}
}
