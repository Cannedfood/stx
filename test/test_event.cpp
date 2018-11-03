#include "catch.hpp"

#include "../event.hpp"

TEST_CASE("Test event", "[event]") {
	struct example_listener : stx::listener<int> {
		int val = -1;
		void on(int i) {
			val = i;
		}
	};

	stx::event<int> event;

	example_listener a, b, c;

	event.add({a, b, c});

	event(42);
	REQUIRE(a.val == 42);
	REQUIRE(b.val == 42);
	REQUIRE(c.val == 42);

	stx::event<int> event2 = std::move(event);
	event2(1337);
	REQUIRE(a.val == 1337);
	REQUIRE(b.val == 1337);
	REQUIRE(c.val == 1337);
}
