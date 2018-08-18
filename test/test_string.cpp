#include "catch.hpp"

#include "../string.hpp"

TEST_CASE("Test string", "[string]") {
	// TODO:

	using stx::string;
	using stx::token;

	string s = "  hello  ";
	CHECK(token("hello") == "hello");
	CHECK(token("  hello  ").trim() == "hello");
}
