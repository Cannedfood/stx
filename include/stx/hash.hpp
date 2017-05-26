#pragma once

#include <cstdint>

namespace stx {

namespace hash_type {

struct fnv_1a_64bit {
	using value_type = uint64_t;
	using state_type = uint64_t;

	constexpr static uint64_t FnvPrime    = 1099511628211UL;
	constexpr static uint64_t OffsetBasis = 14695981039346656037UL;

	constexpr static inline
	state_type create_state() noexcept {
		return OffsetBasis;
	}

	constexpr static inline
	state_type increment_hash(state_type state, const char* data, std::size_t len) noexcept {
		for (std::size_t i = 0; i < len; i++) {
			state ^= data[i];
			state *= FnvPrime;
		}
		return state;
	}

	constexpr static inline
	value_type finalize_hash(const state_type& state) noexcept {
		return state;
	}

	template<std::size_t len> constexpr static inline
	value_type constexpr_hash_strn(const char (&strn)[len]) noexcept {
		state_type state = create_state();
		for(char c : strn) {
			state ^= c;
			state *= FnvPrime;
		}
		return state;
	}
};

struct fnv_1a_32bit {
	using value_type = uint32_t;
	using state_type = uint32_t;

	constexpr static uint32_t FnvPrime    = 16777619U;
	constexpr static uint32_t OffsetBasis = 2166136261U;

	constexpr static inline
	state_type create_state() noexcept {
		return OffsetBasis;
	}

	constexpr static
	state_type increment_hash(state_type state, const char* data, std::size_t len) noexcept {
		for (std::size_t i = 0; i < len; i++) {
			state ^= data[i];
			state *= FnvPrime;
		}
		return state;
	}

	constexpr static inline
	value_type finalize_hash(const state_type& state) noexcept {
		return state;
	}

	template<std::size_t len> constexpr static
	value_type constexpr_hash_strn(const char (&strn)[len]) noexcept {
		state_type state = create_state();
		for(char c : strn) {
			state ^= c;
			state *= FnvPrime;
		}
		return state;
	}
};

struct fnv_1a_32bit_xor_folded {
	using value_type = uint16_t;
	using state_type = typename fnv_1a_32bit::state_type;

	constexpr static inline
	state_type create_state() noexcept {
		return fnv_1a_32bit::create_state();
	}

	constexpr static inline
	state_type increment_hash(const state_type& state, const char* data, std::size_t len) noexcept {
		return fnv_1a_32bit::increment_hash(state, data, len);
	}

	constexpr static inline
	value_type finalize_hash(const state_type& state) noexcept {
		return ((state << 16) ^ state);
	}

	template<std::size_t len> constexpr static inline
	value_type constexpr_hash_strn(const char (&strn)[len]) noexcept {
		value_type val = fnv_1a_32bit::constexpr_hash_strn(strn);
		return ((val << 16) ^ val);
	}
};

using fast64 = fnv_1a_64bit;
using fast32 = fnv_1a_32bit;
using fast   = fnv_1a_64bit;

using default_hash = fast64;

} // namespace hash_type



template<typename hashtype = hash_type::default_hash>
struct basic_hash {
	using value_type = typename hashtype::value_type;
	using state_type = typename hashtype::state_type;

	value_type value;

	inline
	operator value_type() const noexcept { return value; }
};

template<typename hashtype = hash_type::default_hash>
class basic_symbol {
	using Tself = basic_symbol<hashtype>;
	using hash_value = typename hashtype::value_type;

	constexpr inline
	basic_symbol(const char* v, std::size_t len, hash_value h) :
		hash(h),
		value(v),
		length(len)
	{}
public:
	const hash_value  hash;
	const char*       value;
	const std::size_t length;

	template<std::size_t len> constexpr // implicit
	basic_symbol(const char (&strn)[len]) :
		hash(hashtype::constexpr_hash_strn(strn)),
		value(strn),
		length(len)
	{}

	constexpr inline
	static Tself unsafe_construct(const char* data, std::size_t len) {
		auto hash = hashtype::finalize_hash(
			hashtype::increment_hash(
				hashtype::create_state(),
				data, len
			)
		);
		return Tself(data, len, hash);
	}

	constexpr inline
	operator const char*() const noexcept {
		return value;
	}

	constexpr inline
	operator hash_value() const noexcept {
		return hash;
	}
};

using hash   = basic_hash<>;
using symbol = basic_symbol<>;

using hash16   = basic_hash<hash_type::fnv_1a_32bit_xor_folded>;
using symbol16 = basic_symbol<hash_type::fnv_1a_32bit_xor_folded>;

using hash32   = basic_hash<hash_type::fnv_1a_32bit>;
using symbol32 = basic_symbol<hash_type::fnv_1a_32bit>;

using hash64   = basic_hash<hash_type::fnv_1a_64bit>;
using symbol64 = basic_symbol<hash_type::fnv_1a_64bit>;

constexpr inline
symbol operator "" _sym(const char* strn, std::size_t len) {
	return symbol::unsafe_construct(strn, len);
}

} // namespace stx

namespace std {

template<typename T>
struct hash;

template<typename H>
struct hash<::stx::basic_symbol<H>> {
	size_t operator()(const ::stx::basic_symbol<H>& sym) const noexcept {
		return sym.hash;
	}
};

template<typename H>
struct hash<stx::basic_hash<H>> {
	size_t operator()(const stx::basic_hash<H>& h) const noexcept {
		return h.value;
	}
};

} // namespace std
