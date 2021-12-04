#pragma once

template <class T>
class Singleton
{
public:
	Singleton();
	virtual ~Singleton();

	[[nodiscard]] static T& Get();

private:
	static T* _instance;
};

template <class T>
Singleton<T>::Singleton()
{
	if (_instance)
		delete _instance;
	_instance = static_cast<T*>(this);
}

template <class T>
Singleton<T>::~Singleton()
{
	if (_instance == this)
		_instance = nullptr;
}

template <class T>
T& Singleton<T>::Get()
{
	return *_instance;
}

template <class T>
T* Singleton<T>::_instance = nullptr;