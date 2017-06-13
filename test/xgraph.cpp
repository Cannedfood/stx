#define STX_WIP

#include <xgraph>

#include "test.hpp"

using namespace stx;

static
void test_list_element() {
	struct element : public list_element<element> {};

	element head;
	element tail;

	{
		element middle;
		head.insert_as_next(&middle);
		middle.insert_as_next(&tail);

		test(head.last() == nullptr);
		test(head.next() == &middle);
		test(middle.last() == &head);
		test(middle.next() == &tail);
		test(tail.last() == &middle);
		test(tail.next() == nullptr);

		test(tail.head() == &head);
		test(head.tail() == &tail);

		element new_tail;
		element new_head;
		head.push_back(&new_tail);
		tail.push_front(&new_head);

		test(new_tail.head() == &new_head);
		test(new_head.tail() == &new_tail);
	}

	test(head.last() == nullptr);
	test(head.next() == &tail);
	test(tail.last() == &head);
	test(tail.next() == nullptr);

	tail.insert_as_next(&head);
	test(tail.last() == nullptr);
	test(tail.next() == &head);
	test(head.last() == &tail);
	test(head.next() == nullptr);

	tail.insert_as_last(&head);
	test(head.last() == nullptr);
	test(head.next() == &tail);
	test(tail.last() == &head);
	test(tail.next() == nullptr);
}

static
void test_parent_child() {
	struct parent;
	struct child;

	struct parent : public parent_element<parent, child> {};
	struct child  : public child_element <child, parent> {};

	parent p;
	child head;
	child middle;
	child tail;

	p.push_back (&middle);

	p.push_front(&head);
	p.push_back (&tail);

	test(head.parent() == &p);
	test(middle.parent() == &p);
	test(tail.parent() == &p);

	test(middle.next() == &tail);
	test(middle.last() == &head);

	test(p.children() == &head);

	middle.remove();
	test(middle.parent() == nullptr);
	test(middle.next() == nullptr && middle.last() == nullptr);
	test(head.next() == &tail);
}

static
void test_tree() {
	// TODO
}

static
void test_graph() {
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
	test_graph();
	test_pointer_pair();
}
