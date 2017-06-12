#pragma once

#include <cstdint>

#include <utility>
#include <type_traits>

namespace stx {

namespace hash_type {

struct fnv_1a_64bit {
	using value_type = uint64_t;
	using state_type = uint64_t;

	constexpr static const uint64_t FnvPrime    = 1099511628211UL;
	constexpr static const uint64_t OffsetBasis = 14695981039346656037UL;

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
		for (std::size_t i = 0; i < len - 1; i++) { // len - 1 -> last element == '\0'
			state ^= strn[i];
			state *= FnvPrime;
		}
		return finalize_hash(state);
	}
};

struct fnv_1a_32bit {
	using value_type = uint32_t;
	using state_type = uint32_t;

	constexpr static const uint32_t FnvPrime    = 16777619U;
	constexpr static const uint32_t OffsetBasis = 2166136261U;

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
		for (std::size_t i = 0; i < len - 1; i++) { // len - 1 -> last element == '\0'
			state ^= strn[i];
			state *= FnvPrime;
		}
		return finalize_hash(state);
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



template<typename hasher = hash_type::default_hash>
struct basic_hash {
	using type       = basic_hash<hasher>;
	using value_type = typename hasher::value_type;
	using state_type = typename hasher::state_type;

	value_type hash_value;

	constexpr inline
	operator value_type() const noexcept { return hash_value; }

	inline
	basic_hash(const void* data, std::size_t size) :
		hash_value(hasher::finalize_hash(
			hasher::increment_hash(
				hasher::create_state(),
				(const char*) data, size
			)
		))
	{}

	template<typename T, typename = typename std::enable_if<std::is_trivial<T>::value>::type> inline
	basic_hash(T const& t) :
		basic_hash(&t, sizeof(T))
	{}

	template<std::size_t len> constexpr
	basic_hash(const char (&strn)[len]) :
		hash_value(hasher::constexpr_hash_strn(strn))
	{}
};

template<typename hasher = hash_type::default_hash>
class basic_symbol {
	using Tself = basic_symbol<hasher>;
	using hash_value = typename hasher::value_type;

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
		hash(hasher::constexpr_hash_strn(strn)),
		value(strn),
		length(len)
	{}

	constexpr
	static Tself unsafe_construct(const char* data, std::size_t len) {
		return Tself(data, len, basic_hash<hasher>(data, len));
	}

	constexpr
	operator const char*() const noexcept {
		return value;
	}

	constexpr
	operator hash_value() const noexcept {
		return hash;
	}


	bool operator<(const Tself& other) const noexcept {
		if(hash != other.hash) return hash < other.hash;
		for(std::size_t i = 0; i < (length - 1) && i < (other.length - 1); i++) {
			if(value[i] != other.value[i]) return value[i] < other.value[i];
		}
		return false; // same
	}

	bool operator==(const Tself& other) const noexcept {
		if(hash != other.hash) {
			return false;
		}
		else if(value == other.value && length == other.length) {
			return true;
		}
		else {
			for (std::size_t i = 0; i < (length - 1) && i < (other.length - 1); i++) {
				if(value[i] != other.value[i]) return false;
			}
			return true;
		}
	};
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
