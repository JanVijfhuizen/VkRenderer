#pragma once

namespace vi
{
	template <typename T>
	class Vector final : public ArrayPtr<T>
	{
	public:
		Vector();
		explicit Vector(size_t size, FreeListAllocator& allocator);

		void Add(const T& value = {});
		void RemoveAt(size_t index);
		void Clear();

		[[nodiscard]] size_t GetCount() const;

		[[nodiscard]] Iterator<T> end() const override;

	private:
		size_t _count = 0;
	};

	template <typename T>
	Vector<T>::Vector() = default;

	template <typename T>
	Vector<T>::Vector(const size_t size, FreeListAllocator& allocator) : ArrayPtr<T>(size, allocator)
	{
	}

	template <typename T>
	void Vector<T>::Add(const T& value)
	{
		auto allocator = ArrayPtr<T>::GetAllocator();
		assert(allocator);
		const size_t length = ArrayPtr<T>::GetLength();
		if(++_count >= length)
			ArrayPtr<T>::Reallocate(Ut::Max<size_t>(1, length * 2), *allocator);
		ArrayPtr<T>::operator[](_count - 1) = value;
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
