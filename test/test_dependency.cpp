#include "catch.hpp"

#include "../dependency.hpp"

using namespace stx;

TEST_CASE("Dependency injection works", "[dep]") {
	stx::dependencies deps;

	SECTION("Singleton works") {
		deps.singleton<int>();

		deps.invoke((void(*)(shared<int>, shared<int>))
			[](shared<int> a, shared<int> b) -> void {
				REQUIRE(a);
				REQUIRE(b);
				REQUIRE(a == b);
			}
		);
	}

	SECTION("Transient works") {
		deps.transient<int>();

		deps.invoke((void(*)(int, int))
			[](int a, int b) -> void {
				REQUIRE(a);
				REQUIRE(b);
				REQUIRE(a != b);
			}
		);
	}
}
