// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#pragma once

#include <iterator>

namespace stx {

template<class T>
class dlist;

template<class T>
class dlist_iterator;

template<class Derived>
class dlist_element;

/// A doubly-linked list using curiously recurring templates
template<class Derived>
class dlist_element {
public:
	using list_t = dlist<Derived>;
	using element_t = dlist_element<Derived>;

	dlist_element() :
		m_next(nullptr),
		m_prev(nullptr)
	{}
	~dlist_element() {
		remove();
	}

	void push_tail(Derived** d) {
		if(*d)
			static_cast<element_t*>(*d)->insert_next((Derived*) this);
		else
			remove();
		*d = static_cast<Derived*>(this);
	}

	void insert_next(Derived* d) {
		d->remove();
		d->m_prev = (Derived*)this;
		d->m_next = m_next;
		if(m_next)
			m_next->m_prev = d;
		m_next = d;
	}

	void insert_prev(Derived* d) {
		d->remove();
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

	static Derived* next(Derived* d) noexcept {
		return ((element_t*)d)->next();
	}
	static Derived* prev(Derived* d) noexcept {
		return ((element_t*)d)->next();
	}

private:
	Derived* m_next;
	Derived* m_prev;
};

template<class T>
class dlist_iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
	T* m_value;
public:
	using self_t    = dlist_iterator<T>;
	using element_t = dlist_element<T>;

	dlist_iterator(T* p = nullptr) noexcept : m_value(p) {}

	T& operator*()  const { return *m_value; }
	T* operator->() const noexcept { return m_value; }

	self_t& operator++() noexcept {
		m_value = element_t::next(m_value);
		return *this;
	}

	self_t& operator--() noexcept {
		m_value = element_t::prev(m_value);
		return *this;
	}

	self_t operator++(int) const noexcept {
		return { element_t::next(m_value) };
	}

	self_t operator--(int) const noexcept {
		return { element_t::prev(m_value) };
	}
};

template<class T>
class dlist {
public:
	using element_t = dlist_element<T>;
	using list_t    = dlist<T>;

	dlist() noexcept {
		m_sentinel.m_next = static_cast<T*>(&m_sentinel);
		m_sentinel.m_prev = static_cast<T*>(&m_sentinel);
	}

	bool empty() const noexcept { return m_sentinel.m_next == &m_sentinel; }

	T* push_back(T* t)  noexcept { m_sentinel.insert_prev(t); }
	T* push_front(T* t) noexcept { m_sentinel.insert_next(t); }

	T*       front()       noexcept { return m_sentinel->next(); }
	T*       back()        noexcept { return m_sentinel->prev(); }
	T const* front() const noexcept { return m_sentinel->next(); }
	T const* back()  const noexcept { return m_sentinel->prev(); }

	using iterator         = dlist_iterator<T>;
	using reverse_iterator = std::reverse_iterator<dlist_iterator<T>>;
	iterator         begin() { return iterator{front()}; }
	iterator         end()   { return iterator{back()}; }
	reverse_iterator rbegin() { return reverse_iterator{front()}; }
	reverse_iterator rend()   { return reverse_iterator{back()}; }

private:
	element_t m_sentinel;
};

} // namespace stx

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
