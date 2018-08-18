#include "catch.hpp"

#include "../event.hpp"

TEST_CASE("Test event", "[event]") {
	struct example_listener : stx::listener<int> {
		int val = -1;
		void on(int i) {
			val = i;
		}
	};

	stx::event<int> e;

	example_listener a, b, c;

	e.add({a, b, c});

	int my_val = 1;
	e(my_val, [](int v) { return v; });

	CHECK(a.val == my_val);
	CHECK(b.val == my_val);
	CHECK(c.val == my_val);

	my_val = 2;
	c.val = 115;
	e(my_val, [&](int v) { return --my_val > 0; });
	CHECK(a.val == 2);
	CHECK(b.val == 1);
	CHECK(c.val == 115); // left; Predicate returns 0

	stx::event<int> e2 = std::move(e);
}
