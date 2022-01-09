#pragma once

namespace vi
{
	/// <summary>
	/// Hashable data structure that proxies for a c string.
	/// </summary>
	struct CStrRef final
	{
		const char* value = nullptr;
		size_t size = 0;

		CStrRef();
		CStrRef(const char* value);

		[[nodiscard]] size_t operator%(size_t mod) const;
		[[nodiscard]] bool operator==(const CStrRef& other) const;
	};

	inline CStrRef::CStrRef() = default;

	inline CStrRef::CStrRef(const char* value) : value(value), size(Ut::StrLen(value))
	{
	}

	inline size_t CStrRef::operator%(const size_t mod) const
	{
		size_t result = 0;
		const size_t prime = 31;
		for (size_t i = 0; i < size; ++i)
			result = value[i] + result * prime;
		return result % mod;
	}

	inline bool CStrRef::operator==(const CStrRef& other) const
	{
		return strcmp(value, other.value);
	}
}
