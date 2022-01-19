#pragma once

namespace vi
{
	#define PI 3.14159265358979323846f
	#define DEG2_RAD (PI / 180)

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

		static glm::vec2 RotateDegrees(const glm::vec2 v, float degrees);

		static glm::vec2 RotateRadians(const glm::vec2 v, const float radians);

		/// <summary>
		/// Gets the length of a given string.
		/// </summary>
		static size_t StrLen(const char* cStr);
	};

	inline glm::vec2 Ut::RotateDegrees(const glm::vec2 v, float degrees)
	{
		return RotateRadians(v, degrees * DEG2_RAD);
	}

	inline glm::vec2 Ut::RotateRadians(const glm::vec2 v, const float radians)
	{
		const float ca = std::cos(radians);
		const float sa = std::sin(radians);
		return glm::vec2(ca * v.x - sa * v.y, sa * v.x + ca * v.y);
	}

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
