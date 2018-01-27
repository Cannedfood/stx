// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#include "test.hpp"

#include "../graph.hpp"
// #include "../graph_mt.hpp"

#include <algorithm>
#include <vector>
#include <random>

template<class element> static
void test_stxlist() {
	element* head = nullptr;

	element a, b, c, d;
	printf("head*: %p\na: %p\nb:%p\nc:%p\nd:%p\n", &head, &a, &b, &c, &d);

	a.insert_to(head);
	test(head == &a);
	b.insert_to(head);
	test(b.to_this() == &head);
	test(b.next() == &a);

	c.insert_to(head);
	test(c.to_this() == &head);
	test(c.next() == &b);
	c.remove();
	test(c.to_this() == nullptr);
	test(c.next() == nullptr);
	test(head == &b);
	test(b.to_this() == &head);
	test(b.next() == &a);

	a.insert_to(head);
	test(a.to_this() == &head);
	test(a.next() == &b);
	test(b.next() == nullptr);

	// Test move
	test(head == &a);
	test(a.to_this() == &head);
	test(a.next() == &b);
	test(b.to_this() == &a.next());
	test(b.next() == nullptr);

	c = std::move(a);
	test(head == &c);
	test(c.to_this() == &head);
	test(c.next() == &b);
	test(b.to_this() == &c.next());
	test(b.next() == nullptr);

	std::swap(a, c);
	test(head == &a);
	test(a.to_this() == &head);
	test(a.next() == &b);
	test(b.to_this() == &a.next());
	test(b.next() == nullptr);

	b.insert(&c);

	std::swap(a, b);
	test(head == &b);
	test(b.to_this() == &head);
	test(b.next() == &a);
	test(a.to_this() == &b.next());
	test(a.next() == &c);
	test(c.to_this() == &a.next());
}

/* TODO
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

template<class element>
void test_stxtree() {
	element r, a, b, c;
}

// Single threaded list element
struct st_element : public stx::list_element<st_element> {};
// struct single_threaded_tree_element : public stx::tree_element<single_threaded_tree_element> {};

void test_xgraph() {
	test_stxlist<st_element>();
	// test_stxlist_sort();
	// test_stxtree<single_threaded_tree_element>();
	// TODO: Test multi threaded
}

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
