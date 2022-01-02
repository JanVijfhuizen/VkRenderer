#pragma once

namespace vi
{
	template <typename T>
	class UniquePtr final
	{
	public:
		// ReSharper disable once CppNonExplicitConvertingConstructor
		UniquePtr(void* ptr);
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

		UniquePtr<T>& Move(UniquePtr<T>& other);
	};

	template <typename T>
	UniquePtr<T>::UniquePtr(void* ptr) : _ptr(reinterpret_cast<T*>(ptr))
	{
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
		GMEM.Delete(_ptr);
		GMEM_TEMP.Delete(_ptr);
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
		other._ptr = nullptr;
		return *this;
	}
}