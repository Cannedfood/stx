#ifndef STX_HASH_HPP_INCLUDED
#define STX_HASH_HPP_INCLUDED

#pragma once

#include <type_traits>
#include <cstddef>
#include <string_view>

namespace stx {

// TODO: 16bit
template<class Hash = size_t>
struct fnv1a {
	static_assert(
		sizeof(Hash) == 2 || sizeof(Hash) == 4 || sizeof(Hash) == 8,
		"template parameter Hash has to be a uint32_t or a uint64_t"
	);
	static_assert(
		std::is_unsigned_v<Hash>,
		"template parameter Hash has to be a uint16_t, uint32_t or a uint64_t"
	);

	using State = std::conditional_t<(sizeof(Hash) == 2), uint16_t, Hash>;

	constexpr static Hash kFnvBasis =
		sizeof(Hash) == 8 ? 14695981039346656037ul : 2166136261ul;

	constexpr static Hash kFnvPrime =
		sizeof(Hash) == 8 ? 1099511628211ul : 16777619ul;

	Hash m_state = kFnvBasis;
public:
	inline
	fnv1a& operator()(std::byte const* t, size_t count) noexcept {
		for(size_t i = 0; i < count; i++) {
			m_state ^= Hash(t[i]);
			m_state *= kFnvPrime;
		}
		return *this;
	}

	template<class T> inline
	fnv1a& operator()(T const* data, size_t count) {
		return (*this)(reinterpret_cast<std::byte const*>(data), count * sizeof(T));
	}

	inline
	fnv1a& operator()(std::string_view s) {
		return (*this)(s.data(), s.size());
	}

	constexpr
	Hash value() const noexcept {
		if constexpr(sizeof(Hash) == 2)
			return uint16_t(m_state >> 16) xor uint16_t(m_state);
		else
			return m_state;
	}

	constexpr
	operator Hash() const noexcept { return value(); }
};

using hasher = fnv1a<>;

} // namespace stx

#endif // header guard STX_HASH_HPP_INCLUDED
