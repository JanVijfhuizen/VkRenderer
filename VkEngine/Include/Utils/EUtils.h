#pragma once

// Utility methods for the engine.
class EUt final
{
public:
	// Linearly move values in the array until the lowest value is at the front. Loops around.
	template <typename T, typename U>
	static void LinMove(T* arr, U* comparables, size_t from, size_t to);

	// Linearly move values in the array by N steps. Loops around.
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

	// Move the array and comparibles by N spots, where N is the index of the lowest value.
	LinMoveN(arr, from, to, lowest);
	LinMoveN(comparables, from, to, lowest);
}

template <typename T>
void EUt::LinMoveN(T* arr, const size_t from, const size_t to, const size_t amount)
{
	assert(from <= to);

	// Make a temporary array to store the moved values in.
	const size_t length = to - from;
	vi::ArrayPtr<T> tArr{ length, GMEM_TEMP };

	// Assign values in the temporary array before copying it back.
	for (size_t i = 0; i < length; ++i)
		tArr[i] = arr[from + i];

	// Copy yhe temporary values back into the original array.
	for (size_t i = 0; i < length; ++i)
	{
		const uint32_t tIndex = (i + amount) % length;
		arr[from + i] = tArr[tIndex];
	}
}
