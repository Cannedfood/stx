#pragma once

#include <random>

namespace stx {

template<class T>
class basic_guid {
	T m_value;

public:
	inline
	basic_guid() noexcept :
		m_value(generate())
	{}

	constexpr inline
	basic_guid(T value) noexcept :
		m_value(value)
	{}

	constexpr T value() const noexcept { return m_value; }
	constexpr operator T() const noexcept { return value(); }

	constexpr bool operator==(basic_guid const& other) const noexcept { return value() == other.value(); }
	constexpr bool operator!=(basic_guid const& other) const noexcept { return value() != other.value(); }
	constexpr bool operator< (basic_guid const& other) const noexcept { return value() <  other.value(); }
	constexpr bool operator> (basic_guid const& other) const noexcept { return value() >  other.value(); }
	constexpr bool operator<=(basic_guid const& other) const noexcept { return value() <= other.value(); }
	constexpr bool operator>=(basic_guid const& other) const noexcept { return value() >= other.value(); }


	static constexpr basic_guid null() noexcept { return {0}; }

	static T generate() {
		static std::mt19937_64 generator{ std::random_device()() };
		return std::uniform_int_distribution<T>()(generator);
	}
};

using guid = basic_guid<uint64_t>;

} // namespace stx

namespace std {

template<class T>
struct hash<::stx::basic_guid<T>> {
	size_t operator()(::stx::basic_guid<T> const& v) const noexcept {
		return v.value;
	}
};

} // namespace std
