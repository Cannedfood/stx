#pragma once

#include <random>

namespace stx {

template<class T>
struct basic_guid {
	T value;

	basic_guid() noexcept :
		value(generate())
	{}

	constexpr
	basic_guid(T value) noexcept :
		value(value)
	{}

	constexpr operator T() const noexcept { return value; }
	constexpr bool operator==(basic_guid const& other) const noexcept { return value == other.value; }
	constexpr bool operator!=(basic_guid const& other) const noexcept { return value != other.value; }
	constexpr bool operator< (basic_guid const& other) const noexcept { return value <  other.value; }
	constexpr bool operator> (basic_guid const& other) const noexcept { return value >  other.value; }
	constexpr bool operator<=(basic_guid const& other) const noexcept { return value <= other.value; }
	constexpr bool operator>=(basic_guid const& other) const noexcept { return value >= other.value; }

	static constexpr basic_guid null() noexcept { return {0}; }

	static T generate() {
		#if defined (_MSC_VER)  // Visual studio
			#define thread_local __declspec( thread )
		#elif defined (__GCC__) // GCC
			#define thread_local __thread
		#endif

		static thread_local std::mt19937_64 generator{ std::random_device()() };
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
