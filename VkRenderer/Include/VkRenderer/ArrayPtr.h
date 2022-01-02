#pragma once
#include <cstdint>

namespace vi
{
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

		[[nodiscard]] constexpr T& operator[](uint32_t index) const
		{
			return _data[index];
		}

		ArrayPtr();
		explicit ArrayPtr(size_t size, FreeListAllocator& allocator);
		explicit ArrayPtr(void* begin, size_t size);
		ArrayPtr(ArrayPtr<T>& other);
		ArrayPtr(ArrayPtr<T>&& other) noexcept;
		ArrayPtr<T>& operator=(ArrayPtr<T> const& other);
		ArrayPtr<T>& operator=(ArrayPtr<T>&& other) noexcept;
		~ArrayPtr();

		[[nodiscard]] constexpr Iterator begin() const;
		[[nodiscard]] Iterator end() const;

		[[nodiscard]] constexpr bool IsNull() const;
		[[nodiscard]] constexpr size_t GetSize() const;
		[[nodiscard]] constexpr T* GetData() const;

		[[nodiscard]] bool GetHasOwnership() const;

	private:
		T* _data = nullptr;
		size_t _size = 0;
		FreeListAllocator* _allocator = nullptr;

		ArrayPtr<T>& Move(ArrayPtr<T>& other);
	};

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
	bool ArrayPtr<T>::GetHasOwnership() const
	{
		return _allocator;
	}

	template <typename T>
	ArrayPtr<T>& ArrayPtr<T>::Move(ArrayPtr<T>& other)
	{
		_data = other._data;
		_size = other._size;
		_allocator = other._allocator;
		other._data = nullptr;
		other._allocator = nullptr;
		return *this;
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
	ArrayPtr<T>::ArrayPtr(const size_t size, FreeListAllocator& allocator) : _size(size), _allocator(&allocator)
	{
		_data = reinterpret_cast<T*>(allocator.MAlloc(sizeof(T) * size));
	}

	template <typename T>
	ArrayPtr<T>::ArrayPtr(void* begin, const size_t size) :
		_data(reinterpret_cast<T*>(begin)), _size(size)
	{

	}

	template <typename T>
	ArrayPtr<T>::ArrayPtr(ArrayPtr<T>& other)
	{
		Move(other);
	}

	template <typename T>
	ArrayPtr<T>::ArrayPtr(ArrayPtr<T>&& other) noexcept
	{
		Move(other);
	}

	template <typename T>
	ArrayPtr<T>& ArrayPtr<T>::operator=(ArrayPtr<T> const& other)
	{
		return Move(other);
	}

	template <typename T>
	ArrayPtr<T>& ArrayPtr<T>::operator=(ArrayPtr<T>&& other) noexcept
	{
		return Move(other);  // NOLINT(cppcoreguidelines-c-copy-assignment-signature)
	}

	template <typename T>
	ArrayPtr<T>::~ArrayPtr()
	{
		if (_allocator)
			_allocator->MFree(_data);
	}
}
