#pragma once

namespace vi
{
	/// <summary>Smart pointer that manages the lifetime of an object on the heap.</summary>
	template <typename T>
	class UniquePtr final
	{
	public:
		/// <param name="allocator">The allocator that holds the owned object</summary>
		/// <param name="args">Variadic arguments which are passed to the object's constructor.</param>
		template <typename ...Args>
		UniquePtr(FreeListAllocator& allocator, Args... args);
		UniquePtr(UniquePtr<T>& other);
		UniquePtr(UniquePtr<T>&& other) noexcept;
		UniquePtr<T>& operator=(UniquePtr<T> const& other);
		UniquePtr<T>& operator=(UniquePtr<T>&& other) noexcept;
		~UniquePtr();

		// ReSharper disable once CppNonExplicitConversionOperator
		operator T* () const;
		operator bool() const;
	private:
		T* _ptr = nullptr;
		FreeListAllocator* _allocator = nullptr;

		UniquePtr<T>& Move(UniquePtr<T>& other);
	};

	template <typename T>
	template <typename ... Args>
	UniquePtr<T>::UniquePtr(FreeListAllocator& allocator, Args... args) : _allocator(&allocator)
	{
		_ptr = allocator.New<T>(args);
	}

	template <typename T>
	UniquePtr<T>::UniquePtr(UniquePtr<T>& other)
	{
		Move(other);
	}

	template <typename T>
	UniquePtr<T>::UniquePtr(UniquePtr<T>&& other) noexcept
	{
		Move(other);
	}

	template <typename T>
	UniquePtr<T>& UniquePtr<T>::operator=(UniquePtr<T> const& other)
	{
		return Move(other);
	}

	template <typename T>
	UniquePtr<T>& UniquePtr<T>::operator=(UniquePtr<T>&& other) noexcept
	{
		return Move(other);
	}

	template <typename T>
	UniquePtr<T>::~UniquePtr()
	{
		if(_allocator)
			_allocator->Delete(_ptr);
	}

	template <typename T>
	UniquePtr<T>::operator T* () const
	{
		return _ptr;
	}

	template <typename T>
	UniquePtr<T>::operator bool() const
	{
		return _ptr != nullptr;
	}

	template <typename T>
	UniquePtr<T>& UniquePtr<T>::Move(UniquePtr<T>& other)
	{
		_ptr = other._ptr;
		_allocator = other._allocator;
		other._ptr = nullptr;
		return *this;
	}
}