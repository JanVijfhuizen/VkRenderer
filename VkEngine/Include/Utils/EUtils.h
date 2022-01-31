#pragma once

class EUt final
{
public:
	template <typename T, typename U>
	static void LinMove(T* arr, U* comparables, size_t from, size_t to);

	template <typename T>
	static void LinMoveN(T* arr, size_t from, size_t to, size_t amount);
};

template <typename T, typename U>
void EUt::LinMove(T* arr, U* comparables, const size_t from, const size_t to)
{
	assert(from <= to);

	const size_t length = to - from;

	// Find instance that should be at index 0.
	uint32_t lowest = 0;
	for (size_t i = 1; i < length; ++i)
		if (comparables[from + i] < comparables[from + lowest])
			lowest = i;

	// If the array is already properly ordered.
	if (lowest == 0)
		return;

	LinMoveN(arr, from, to, lowest);
	LinMoveN(comparables, from, to, lowest);
}

template <typename T>
void EUt::LinMoveN(T* arr, const size_t from, const size_t to, const size_t amount)
{
	assert(from <= to);

	const size_t length = to - from;
	vi::ArrayPtr<T> tArr{ length, GMEM_TEMP };

	for (size_t i = 0; i < length; ++i)
		tArr[i] = arr[from + i];

	for (size_t i = 0; i < length; ++i)
	{
		const uint32_t tIndex = (i + amount) % length;
		arr[from + i] = tArr[tIndex];
	}
}
