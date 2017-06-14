#include <xhash>

#include "test.hpp"

using namespace stx;

template<typename hasher> static
void test_hasher() {
	// TODO: test hasher

	// Make the constexpr_hash_strn(text) yields the same result as the runtime hash
	const char text[]    = "Hello";
	std::size_t text_len = 5;
	test(hasher::constexpr_hash_strn(text) == basic_hash<hasher>(text, text_len));
}

template<typename sym> static
void test_symbol() {
	sym a = "Hi";
	sym b = "Hi";
	sym c = "Ho";
	test(a.hash() == b.hash());
	test(b.hash() != c.hash());
}

void test_xhash() {
	test_hasher<hash_type::fnv_1a_32bit_xor_folded>();
	test_hasher<hash_type::fnv_1a_32bit>();
	test_hasher<hash_type::fnv_1a_64bit>();

	test_symbol<symbol16>();
	test_symbol<symbol32>();
	test_symbol<symbol64>();

	test_symbol<symstring16>();
	test_symbol<symstring32>();
	test_symbol<symstring64>();
}
