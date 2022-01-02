#pragma once

class Ut final
{
public:
	template <typename T>
	static T Min(const T& a, const T& b);
	template <typename T>
	static T Max(const T& a, const T& b);
	template <typename T>
	static T Clamp(const T& t, const T& min, const T& max);
	template <typename T>
	static T Lerp(const T& t, const T& a, const T& b);
};

template <typename T>
T Ut::Min(const T& a, const T& b)
{
	return a < b ? a : b;
}

template <typename T>
T Ut::Max(const T& a, const T& b)
{
	return a > b ? a : b;
}

template <typename T>
T Ut::Clamp(const T& t, const T& min, const T& max)
{
	const T r = t < max ? t : max;
	return r > min ? r : min;
}

template <typename T>
T Ut::Lerp(const T& t, const T& a, const T& b)
{
	return a + t * (b - a);
}
