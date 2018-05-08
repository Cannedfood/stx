#include "test.hpp"

#include "../event.hpp"

struct example_listener : stx::listener<int> {
	int val = -1;
	void on(int i) {
		val = i;
	}
};

void test_event() {
	stx::event<int> e;

	example_listener a, b, c;

	e.add({a, b, c});

	int my_val = 1;
	e(my_val, [](int v) { return v; });

	test(a.val == my_val);
	test(b.val == my_val);
	test(c.val == my_val);

	my_val = 2;
	c.val = 115;
	e(my_val, [&](int v) { return --my_val > 0; });
	test(a.val == 2);
	test(b.val == 1);
	test(c.val == 115); // left; Predicate returns 0

	stx::event<int> e2 = std::move(e);
}
