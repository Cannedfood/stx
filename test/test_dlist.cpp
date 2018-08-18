#include "catch.hpp"

#include "../dlist.hpp"

TEST_CASE("Test dlist", "[dlist]") {
	struct test_elem : public stx::dlist_element<test_elem> {
		char c;

		test_elem(char c) : c(c) {}
	};

	test_elem
		a = {'a'},
		b = {'b'},
		c = {'c'},
		d = {'d'};

	stx::dlist<test_elem> elem;
	elem.push_back(&a);
	elem.push_back(&b);
	elem.push_back(&c);
	elem.push_back(&d);

	CHECK(a.next() == &b);
	CHECK(b.next() == &c);
	CHECK(c.next() == &d);
	CHECK(d.prev() == &c);
	CHECK(c.prev() == &b);
	CHECK(b.prev() == &a);

	b.remove();

	CHECK(b.next() == nullptr);
	CHECK(b.prev() == nullptr);

	CHECK(a.next() == &c);
	CHECK(c.next() == &d);
	CHECK(d.prev() == &c);
	CHECK(c.prev() == &a);
}
