#pragma once

namespace vi
{
	struct StringHashable final
	{
		const char* value = nullptr;
		size_t size = 0;

		StringHashable();
		StringHashable(const char* value);

		[[nodiscard]] size_t operator%(size_t mod) const;
		[[nodiscard]] bool operator==(const StringHashable& other) const;
	};

	inline StringHashable::StringHashable() = default;

	inline StringHashable::StringHashable(const char* value) : value(value), size(Ut::StrLen(value))
	{
	}

	inline size_t StringHashable::operator%(const size_t mod) const
	{
		size_t result = 0;
		const size_t prime = 31;
		for (size_t i = 0; i < size; ++i)
			result = value[i] + result * prime;
		return result % mod;
	}

	inline bool StringHashable::operator==(const StringHashable& other) const
	{
		if (size != other.size)
			return false;
		for (size_t i = 0; i < size; ++i)
		{
			if (value[i] != other.value[i])
				return false;
		}

		return true;
	}
}
