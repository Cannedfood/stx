#pragma once

#include <cstdint>

#include <utility>
#include <type_traits>
#include <cstring>

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

	constexpr
	basic_hash() :
		hash_value(hasher::finalize_hash(hasher::create_state()))
	{}
};

template<typename hasher = hash_type::default_hash>
class basic_symbol {
	using Tself = basic_symbol<hasher>;
	using hash_value = typename hasher::value_type;

protected:
	hash_value  m_hash;
	const char* m_value;
	std::size_t m_length;

	constexpr inline
	basic_symbol(const char* v, std::size_t len, hash_value h) :
		m_hash(h),
		m_value(v),
		m_length(len)
	{}

public:
	template<std::size_t len> constexpr // implicit
	basic_symbol(const char (&strn)[len]) :
		m_hash(hasher::constexpr_hash_strn(strn)),
		m_value(strn),
		m_length(len)
	{}

	constexpr
	static Tself unsafe_construct(const char* data, std::size_t len) {
		return Tself(data, len, basic_hash<hasher>(data, len));
	}

	constexpr const char* value()  const noexcept { return m_value; }
	constexpr hash_value  hash()   const noexcept { return m_hash; }
	constexpr std::size_t length() const noexcept { return m_length; }

	constexpr
	operator const char*() const noexcept { return value(); }

	constexpr
	operator hash_value() const noexcept { return hash(); }

	bool operator<(const Tself& other) const noexcept {
		if(m_hash != other.m_hash) return m_hash < other.m_hash;
		for(std::size_t i = 0; i < (m_length - 1) && i < (other.m_length - 1); i++) {
			if(m_value[i] != other.m_value[i]) return m_value[i] < other.m_value[i];
		}
		return false; // same
	}

	bool operator==(const Tself& other) const noexcept {
		if(m_hash != other.m_hash) {
			return false;
		}
		else if(m_value == other.m_value && m_length == other.m_length) {
			// Pointers are the same -> Probably string pooling
			return true;
		}
		else {
			for (std::size_t i = 0; i < (m_length - 1) && i < (other.m_length - 1); i++) {
				if(m_value[i] != other.m_value[i]) return false;
			}
			return true;
		}
	};
};

template<typename hasher = hash_type::default_hash>
class basic_symstring : public basic_symbol<hasher> {
	static const char* clone_string(const char* c, std::size_t len) {
		char* cc = new char[len + 1];
		memcpy(cc, c, len);
		cc[len] = '\0';
		return cc;
	}

public:
	basic_symstring() : basic_symbol<hasher>("", 0) {}
	basic_symstring(const char* str) :
		basic_symstring(str, strlen(str))
	{}
	basic_symstring(const char* str, std::size_t len) :
		basic_symbol<hasher>(clone_string(str, len), len, basic_hash<hasher>(str, len))
	{}
	basic_symstring(basic_symbol<hasher> const& s) :
		basic_symbol<hasher>(clone_string(s.value(), s.length()), s.length(), s.hash())
	{}
	~basic_symstring() {
		if(this->m_value) {
			delete[] this->m_value;
		}
	}

	// -- Copy --------------------------------------------------------------
	basic_symstring(basic_symstring<hasher> const& other) :
		basic_symbol<hasher>(clone_string(other.value(), other.length()), other.length(), other.hash())
	{}

	basic_symstring<hasher>& operator=(basic_symstring<hasher> const& other) {
		if(this->m_value) {
			delete[] this->m_value;
		}
		this->value  = clone_string(other.value(), other.length());
		this->length = other.length();
		this->hash   = other.hash();
	}

	// -- Move -------------------------------------------------------------
	basic_symstring(basic_symstring<hasher>&& other) :
		basic_symbol<hasher>(other.m_value, other.m_length, other.m_hash)
	{
		other.m_value  = nullptr;
		other.m_length = 0;
		other.m_hash   = basic_hash<hasher>();
	}

	basic_symstring<hasher>& operator=(basic_symstring<hasher>&& other) {
		if(this->m_value) {
			delete[] this->m_value;
		}

		this->m_value  = other.m_value;
		this->m_length = other.m_length;
		this->m_hash   = other.m_hash;

		other.m_value  = nullptr;
		other.m_length = 0;
		other.m_hash   = basic_hash<hasher>();
	}

	// -- Functionality -------------------------------------------------------

	void reset() {
		if(this->value) {
			this->value  = nullptr;
			this->length = 0;
			this->hash   = basic_hash<hasher>();
		}
	}

	void reset(const char* str) {
		reset(str, strlen(str));
	}

	void reset(const char* str, size_t len) {
		if(this->value) {
			delete[] this->value;
		}
		this->value  = clone_string(str, len);
		this->length = len;
		this->hash   = basic_hash<hasher>(str, len);
	}
};


using hash      = basic_hash<>;
using symbol    = basic_symbol<>;
using symstring = basic_symstring<>;

using hash16      = basic_hash<hash_type::fnv_1a_32bit_xor_folded>;
using symbol16    = basic_symbol<hash_type::fnv_1a_32bit_xor_folded>;
using symstring16 = basic_symstring<hash_type::fnv_1a_32bit_xor_folded>;

using hash32      = basic_hash<hash_type::fnv_1a_32bit>;
using symbol32    = basic_symbol<hash_type::fnv_1a_32bit>;
using symstring32 = basic_symstring<hash_type::fnv_1a_32bit>;

using hash64      = basic_hash<hash_type::fnv_1a_64bit>;
using symbol64    = basic_symbol<hash_type::fnv_1a_64bit>;
using symstring64 = basic_symstring<hash_type::fnv_1a_64bit>;

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
