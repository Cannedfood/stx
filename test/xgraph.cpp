#define STX_WIP

#include <xgraph>

#include "test.hpp"

using namespace stx;

static
void test_list_element() {
	// TODO
}

static
void test_parent_child() {
	// TODO
}

static
void test_tree() {
	// TODO
}

static
void test_pointer_pair() {
	struct twin : public pointer_pair<twin> {};

	twin a;
	twin c;

	test(!a.get());

	{
		twin b;
		a.reset(&b);
		test(b.get() == &a);
		test(a.get() == &b);
		test(c.get() == nullptr);
	}

	test(a.get() == nullptr);

	{
		twin b;
		a.reset(&b);
		c = std::move(a);
		test(a.get() == nullptr);
		test(c.get() == &b);
		test(b.get() == &c);
	}
}

void test_xgraph() {
	test_list_element();
	test_parent_child();
	test_tree();
	test_pointer_pair();
}
