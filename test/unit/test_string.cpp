#include "catch.hpp"

#include <stx/string.hpp>

TEST_CASE("Test string", "[string]") {
	// TODO:

	using stx::string;
	using stx::token;

	string s = "  hello  ";
	CHECK(token("hello") == "hello");
	CHECK(token("  hello  ").trim() == "hello");
}
