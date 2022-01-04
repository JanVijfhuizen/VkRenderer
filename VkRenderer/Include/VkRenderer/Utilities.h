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
		static T Min(const T& a, const T& b);
		/// <returns>Maximum of the two values.</returns>
		template <typename T>
		static T Max(const T& a, const T& b);
		/// <summary>
		/// Clamps value t in between the range of min and max.
		/// </summary>
		template <typename T>
		static T Clamp(const T& t, const T& min, const T& max);
		/// <summary>
		/// Linearly interpolates between a and b by t, where t is unclamped.
		/// </summary>
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
}