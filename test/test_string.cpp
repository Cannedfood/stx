#include "test.hpp"

#include "../string.hpp"

void test_string() {
	// TODO:

	using stx::string;
	using stx::token;

	string s = "  hello  ";
	test(token("hello") == "hello");
	test(token("  hello  ").trim() == "hello");
}
