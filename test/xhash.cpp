#include <xhash>

#include "test.hpp"

using namespace stx;

template<typename hasher> static
void test_hasher() {
	// TODO: test hashers
}

template<typename sym> static
void test_symbol() {
	sym a = "Hi";
	sym b = "Hi";
	sym c = "Ho";
	test(a.hash == b.hash);
	test(b.hash != c.hash);
}

void test_xhash() {
	test_hasher<hash_type::fnv_1a_32bit_xor_folded>();
	test_hasher<hash_type::fnv_1a_32bit>();
	test_hasher<hash_type::fnv_1a_64bit>();

	test_symbol<symbol16>();
	test_symbol<symbol32>();
	test_symbol<symbol64>();
}
