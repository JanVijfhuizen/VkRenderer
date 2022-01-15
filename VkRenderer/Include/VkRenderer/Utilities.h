#pragma once

namespace vi
{
	/// <summary>
	/// Utility class which contains common mathematical functions.
	/// </summary>
	class Ut final
	{
	public:
		/// <returns>Minimum of the two values.</returns>
		template <typename T>
		[[nodiscard]] static T Min(const T& a, const T& b);
		/// <returns>Maximum of the two values.</returns>
		template <typename T>
		[[nodiscard]] static T Max(const T& a, const T& b);
		/// <summary>
		/// Clamps value t in between the range of min and max.
		/// </summary>
		template <typename T>
		[[nodiscard]] static T Clamp(const T& t, const T& min, const T& max);
		/// <summary>
		/// Linearly interpolates between a and b by t, where t is unclamped.
		/// </summary>
		template <typename T>
		[[nodiscard]] static T Lerp(const T& t, const T& a, const T& b);

		static size_t StrLen(const char* cStr);
	};

	inline size_t Ut::StrLen(const char* cStr)
	{
		size_t result = 0;
		while (cStr[result++] != '\0')
			;
		return result;
	}

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
}
