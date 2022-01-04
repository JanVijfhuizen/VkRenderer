#pragma once
#include <cstdint>

namespace vi
{
	/// <summary>
	/// Container class that points to a memory range. If given an allocator, it can own that memory range.
	/// </summary>
	template <typename T>
	class ArrayPtr
	{
	public:
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

		[[nodiscard]] constexpr T& operator[](uint32_t index) const
		{
			return _data[index];
		}

		ArrayPtr();
		/// <summary>Create the array as the owner of a memory range.</summary>
		explicit ArrayPtr(size_t size, FreeListAllocator& allocator, const T& initValue = {});
		/// <summary>Create the array as an observer to a memory range owned by something else.</summary>
		explicit ArrayPtr(void* begin, size_t size);

		ArrayPtr(ArrayPtr<T>& other);
		ArrayPtr(ArrayPtr<T>&& other) noexcept;
		ArrayPtr<T>& operator=(ArrayPtr<T> const& other);
		ArrayPtr<T>& operator=(ArrayPtr<T>&& other) noexcept;
		~ArrayPtr();

		[[nodiscard]] constexpr Iterator begin() const;
		[[nodiscard]] Iterator end() const;

		/// <returns>If data is a nullptr.</returns> 
		[[nodiscard]] constexpr bool IsNull() const;
		/// <returns>Length of the memory range.</returns> 
		[[nodiscard]] constexpr size_t GetLength() const;
		/// <returns>Raw data pointer to memory range.</returns> 
		[[nodiscard]] constexpr T* GetData() const;
		/// <returns>If the array owns the data.</returns> 
		[[nodiscard]] bool GetHasOwnership() const;
		/// <returns>Used allocator, if any.</returns>
		[[nodiscard]] FreeListAllocator* GetAllocator() const;

		/// <summary>Copy a range of values into this array.</summary>
		/// <param name="other">Other array from which to copy the values.</param>  
		/// <param name="begin">Start of the memory range (inclusive).</param>  
		/// <param name="end">End of the memory range (exclusive). 
		/// If given a negative number it is treated as the minimum of both array's lengths.</param>  
		/// <param name="otherBegin">Beginning index for the other array to copy the values from.</param>
		void CopyData(const ArrayPtr<T>& other, uint32_t begin = 0, int32_t end = -1, uint32_t otherBegin = 0) const;
		/// <summary>Reallocate the values to a new memory location. If this owns the previous memory range, deallocate it.</summary>
		/// <param name="length">The new length of the array. If it's smaller than before, only copies the values in that range.</param>  
		/// <param name="allocator">The allocator that holds the new memory range.</param>  
		void Reallocate(size_t length, FreeListAllocator& allocator);
		/// <summary>Resets all values in a given range.</summary>
		/// <param name="start">Start of the memory range (inclusive).</param>  
		/// <param name="end">End of the memory range (exclusive).</param>  
		/// <param name="initValue">The value to reset the everything to.</param>  
		void ResetValues(uint32_t start, int32_t end, const T& initValue = {});

		/// <summary>If the memory is owned, deallocate it. Then reset all values.</summary>
		void Free();

	private:
		T* _data = nullptr;
		size_t _length = 0;
		FreeListAllocator* _allocator = nullptr;

		ArrayPtr<T>& Move(ArrayPtr<T>& other);
	};

	template <typename T>
	constexpr typename ArrayPtr<T>::Iterator ArrayPtr<T>::begin() const
	{
		Iterator it{};
		it.begin = _data;
		it.length = _length;
		it.index = 0;

		return it;
	}

	template <typename T>
	constexpr bool ArrayPtr<T>::IsNull() const
	{
		return GetLength() == 0;
	}

	template <typename T>
	constexpr size_t ArrayPtr<T>::GetLength() const
	{
		return _length;
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
		it.length = _length;
		it.index = _length;

		return it;
	}

	template <typename T>
	bool ArrayPtr<T>::GetHasOwnership() const
	{
		return _allocator;
	}

	template <typename T>
	FreeListAllocator* ArrayPtr<T>::GetAllocator() const
	{
		return _allocator;
	}

	template <typename T>
	void ArrayPtr<T>::CopyData(const ArrayPtr<T>& other, const uint32_t begin, int32_t end, const uint32_t otherBegin) const
	{
		end = end > -1 ? end : other.GetLength();
		const uint32_t length = end - begin;

		assert(begin <= end);
		assert(_length >= end);
		assert(other._length >= otherBegin + length);

		memcpy(&_data[begin], &other._data[otherBegin], sizeof(T) * length);
	}

	template <typename T>
	void ArrayPtr<T>::Reallocate(const size_t length, FreeListAllocator& allocator)
	{
		T* data = reinterpret_cast<T*>(allocator.MAlloc(sizeof(T) * length));
		const uint32_t end = Ut::Min(length, _length);
		memcpy(data, _data, sizeof(T) * end);

		Free();

		_data = data;
		_length = length;
		_allocator = &allocator;

		ResetValues(end, _length);
	}

	template <typename T>
	void ArrayPtr<T>::ResetValues(const uint32_t start, int32_t end, const T& initValue)
	{
		assert(start < end);
		assert(end <= static_cast<int32_t>(_length));

		end = end > -1 ? end : _length;
		for (uint32_t i = start; i < _length; ++i)
			_data[i] = initValue;
	}

	template <typename T>
	void ArrayPtr<T>::Free()
	{
		if (_allocator)
			_allocator->MFree(_data);

		_data = nullptr;
		_length = 0;
		_allocator = nullptr;
	}

	template <typename T>
	ArrayPtr<T>& ArrayPtr<T>::Move(ArrayPtr<T>& other)
	{
		_data = other._data;
		_length = other._length;
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
		Iterator temp(begin, length, index);
		++index;
		return temp;
	}

	template <typename T>
	ArrayPtr<T>::ArrayPtr() = default;

	template <typename T>
	ArrayPtr<T>::ArrayPtr(const size_t size, FreeListAllocator& allocator, const T& initValue) : _length(size), _allocator(&allocator)
	{
		_data = reinterpret_cast<T*>(allocator.MAlloc(sizeof(T) * size));
		ResetValues(0, _length, initValue);
	}

	template <typename T>
	ArrayPtr<T>::ArrayPtr(void* begin, const size_t size) :
		_data(reinterpret_cast<T*>(begin)), _length(size)
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
		Free();
	}
}
