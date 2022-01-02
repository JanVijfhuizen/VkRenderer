#pragma once

template <typename T>
class UniquePtr final
{
public:
	// ReSharper disable once CppNonExplicitConvertingConstructor
	UniquePtr(void* ptr);
	UniquePtr(UniquePtr<T>& other);
	UniquePtr(UniquePtr<T>&& other) noexcept;
	UniquePtr<T>& operator= (UniquePtr<T> const& other);
	UniquePtr<T>& operator=(UniquePtr<T>&& other) noexcept;
	~UniquePtr();

	// ReSharper disable once CppNonExplicitConversionOperator
	operator T*() const;
	operator bool() const;
private:
	T* _ptr = nullptr;
};

template <typename T>
UniquePtr<T>::UniquePtr(void* ptr) : _ptr(reinterpret_cast<T*>(ptr))
{
}

template <typename T>
UniquePtr<T>::UniquePtr(UniquePtr<T>& other)
{
	_ptr = other._ptr;
	other._ptr = nullptr;
}

template <typename T>
UniquePtr<T>::UniquePtr(UniquePtr<T>&& other) noexcept
{
	_ptr = other._ptr;
	other._ptr = nullptr;
}

template <typename T>
UniquePtr<T>& UniquePtr<T>::operator=(UniquePtr<T> const& other)
{
	_ptr = other._ptr;
	other._ptr = nullptr;
	return *this;
}

template <typename T>
UniquePtr<T>& UniquePtr<T>::operator=(UniquePtr<T>&& other) noexcept
{
	_ptr = other._ptr;
	other._ptr = nullptr;
	return *this;
}

template <typename T>
UniquePtr<T>::~UniquePtr()
{
	GMEM.MFree(_ptr);
	GMEM_TEMP.MFree(_ptr);	
}

template <typename T>
UniquePtr<T>::operator T*() const
{
	return _ptr;
}

template <typename T>
UniquePtr<T>::operator bool() const
{
	return _ptr != nullptr;
}
