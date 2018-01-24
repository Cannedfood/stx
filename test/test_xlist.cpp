// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#include "test.hpp"

#include <stx/list.hpp>
#include <stx/list_mt.hpp>

#include <algorithm>

template<class element> static
void test_xlist() {
	element* head = nullptr;

	element a, b, c, d;
	a.add_to(head);
	test(head == &a);
	b.add_to(head);
	test(b.to_this() == &head);
	test(b.next() == &a);

	c.add_to(head);
	test(c.to_this() == &head);
	test(c.next() == &b);
	c.remove();
	test(c.to_this() == nullptr);
	test(c.next() == nullptr);
	test(head == &b);
	test(b.to_this() == &head);
	test(b.next() == &a);

	a.add_to(head);
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

	c.add_to(b);

	std::swap(a, b);
	test(head == &b);
	test(b.to_this() == &head);
	test(b.next() == &a);
	test(a.to_this() == &b.next());
	test(a.next() == &c);
	test(c.to_this() == &a.next());

	// Order to a, b, c, d
	while(head) head->remove();
	d.add_to(head);
	c.add_to(head);
	b.add_to(head);
	a.add_to(head);

	// Test center with even count
	test(head->center() == &b);

	// Test center with uneven count
	test(head->center(&d) == &b);

	while(head) head->remove();
	d.add_to(head);
	c.add_to(head);
	b.add_to(head);
	a.add_to(head);
	test(head->back() == &d);
}

struct single_threaded_element : public stx::list_element<single_threaded_element> {};

void test_xlist() {
	test_xlist<single_threaded_element>();
	// TODO: Test multi threaded
}

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
