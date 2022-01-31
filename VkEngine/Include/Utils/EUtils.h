#pragma once

class EUt final
{
public:
	template <typename T, typename U>
	static void LinMove(T* arr, U* comparables, const size_t from, const size_t to);
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

	vi::ArrayPtr<T> tArr{length, GMEM_TEMP};
	vi::ArrayPtr<U> tComparables{length, GMEM_TEMP};

	for (size_t i = 0; i < length; ++i)
	{
		tArr[i] = arr[from + i];
		tComparables[i] = comparables[from + i];
	}

	for (size_t i = 0; i < length; ++i)
	{
		const uint32_t tIndex = (i + lowest) % length;
		arr[from + i] = tArr[tIndex];
		comparables[from + i] = tComparables[tIndex];
	}
}
