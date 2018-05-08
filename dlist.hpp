// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#pragma once

namespace stx {

/// A doubly-linked list using curiously recurring templates
template<class Derived>
class dlist {
public:
	dlist() :
		m_next(nullptr),
		m_prev(nullptr)
	{}
	~dlist() {
		remove();
	}

	void insert_next(Derived* d) {
		d->m_prev = (Derived*)this;
		d->m_next = m_next;
		if(m_next)
			m_next->m_prev = d;
		m_next = d;
	}

	void insert_prev(Derived* d) {
		d->m_next = (Derived*)this;
		d->m_prev = m_prev;
		if(m_prev)
			m_prev->m_next = d;
		m_prev = d;
	}

	void remove() {
		if(m_next)
			m_next->m_prev = m_prev;
		if(m_prev)
			m_prev->m_next = m_next;
		m_next = m_prev = nullptr;
	}

	Derived* next() noexcept { return m_next; }
	Derived* prev() noexcept { return m_prev; }

private:
	Derived* m_next;
	Derived* m_prev;
};

} // namespace stx

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
