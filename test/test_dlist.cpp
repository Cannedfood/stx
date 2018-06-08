#include "test.hpp"

#include "../dlist.hpp"

struct test_elem : public stx::dlist_element<test_elem> {
	char c;

	test_elem(char c) : c(c) {}
};

void test_dlist() {
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

	test_eq(a.next(), &b);
	test_eq(b.next(), &c);
	test_eq(c.next(), &d);
	test_eq(d.prev(), &c);
	test_eq(c.prev(), &b);
	test_eq(b.prev(), &a);

	b.remove();

	test_eq(b.next(), nullptr);
	test_eq(b.prev(), nullptr);

	test_eq(a.next(), &c);
	test_eq(c.next(), &d);
	test_eq(d.prev(), &c);
	test_eq(c.prev(), &a);
}