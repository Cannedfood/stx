// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#include "catch.hpp"

#include "../list.hpp"
// #include "../list_mt.hpp"

#include <algorithm>
#include <vector>
#include <random>

TEST_CASE("Test stx list - single threaded", "[graph]") {
	// Single threaded list element
	struct element : public stx::list_element<element> {
		int value;
		element() : value(0) {}
		element(int val) : value(val) {}

		bool operator<(element const& other) const noexcept {
			return value < other.value;
		}
	};

	{
		element* head = nullptr;
		element a, b, c, d;

		a.insert_to(head);
		CHECK(head == &a);
		b.insert_to(head);
		CHECK(b.to_this() == &head);
		CHECK(b.next()    == &a);

		c.insert_to(head);
		CHECK(c.to_this() == &head);
		CHECK(c.next()    == &b);
		c.remove();
		CHECK(c.to_this() == nullptr);
		CHECK(c.next()    == nullptr);
		CHECK(head        == &b);
		CHECK(b.to_this() == &head);
		CHECK(b.next()    == &a);

		a.insert_to(head);
		CHECK(a.to_this() == &head);
		CHECK(a.next()    == &b);
		CHECK(b.next()    == nullptr);

		// Test move
		CHECK(head        == &a);
		CHECK(a.to_this() == &head);
		CHECK(a.next()    == &b);
		CHECK(b.to_this() == &a.next());
		CHECK(b.next()    == nullptr);

		c = std::move(a);
		CHECK(head        == &c);
		CHECK(c.to_this() == &head);
		CHECK(c.next()    == &b);
		CHECK(b.to_this() == &c.next());
		CHECK(b.next()    == nullptr);

		std::swap(a, c);
		CHECK(head        == &a);
		CHECK(a.to_this() == &head);
		CHECK(a.next()    == &b);
		CHECK(b.to_this() == &a.next());
		CHECK(b.next()    == nullptr);

		b.insert(&c);

		std::swap(a, b);
		CHECK(head        == &b);
		CHECK(b.to_this() == &head);
		CHECK(b.next()    == &a);
		CHECK(a.to_this() == &b.next());
		CHECK(a.next()    == &c);
		CHECK(c.to_this() == &a.next());
	}

	// Test predicate insert
	if(false) // TODO: Test predicate insert after implementing it
	{
		element a = 1, b = 2, c = 3, d = 4, e = 5;

		element* head = nullptr;
		e.insert_to(head, std::less<>());
		a.insert_to(head, std::less<>());
		d.insert_to(head, std::less<>());
		b.insert_to(head, std::less<>());
		c.insert_to(head, std::less<>());

		/*
		printf(
			"head*: %p\na: %p\nb: %p\nc: %p\nd: %p\ne: %p\n",
			&head, &a, &b, &c, &d, &e
		);
		printf(
			"head: %p\na.next(): %p\nb.next(): %p\nc.next(): %p\nd.next(): %p\ne.next(): %p\n",
			head, a.next(), b.next(), c.next(), d.next(), e.next()
		);
		*/

		CHECK(head     == &a);
		CHECK(a.next() == &b);
		CHECK(b.next() == &c);
		CHECK(c.next() == &d);
		CHECK(d.next() == &e);
		CHECK(e.next() == nullptr);
	}
}

/* TODO: Fails
static
void test_stxlist_sort() {
	struct my_element : public stx::list_element<my_element> {
		unsigned value;
		bool operator<(my_element const& other) const noexcept {
			return value < other.value;
		}
	};

	std::vector<my_element> elements(2048);
	my_element* list = nullptr;
	{
		std::mt19937 gen;
		std::uniform_int_distribution<unsigned> dist;
		for(auto& e : elements) {
			e.value = dist(gen);
			e.insert_to(list);
		}
	}

	// stx::merge_sort(list, std::less<>());

	test(std::is_sorted(list->begin(), list->end(), std::less<>()));
}
*/

// struct single_threaded_tree_element : public stx::tree_element<single_threaded_tree_element> {};

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
