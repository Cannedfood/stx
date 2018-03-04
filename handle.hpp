// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#ifndef STX_HANDLE_HPP_INCLUDED
#define STX_HANDLE_HPP_INCLUDED

#include "list_mt.hpp"

#pragma once

namespace stx {

// TODO: Replace list_element with list_element_mt
class handle_slot : private list_element_mt<handle_slot> {
	friend list_element_t;
	friend list_t;
	friend class handle;
protected:
	handle_slot(handle_slot&&) {}
	handle_slot(handle_slot const&) {}
	void invalidate() { list_element_t::remove(); }
	virtual void on_force_remove() {}
public:
	handle_slot() {}

	void force_remove() {
		if(remove()) { // Try to remove this
			// !! Don't call on_force_remove when we this was already removed (multithreading) !!
			on_force_remove();
		}
	}
};

class handle : public handle_slot {
	list<handle_slot> m_handles;

	void on_force_remove() override { clear(); }
public:
	handle() {}
	handle(handle_slot& slot) { m_handles.add(&slot); }
	handle(std::initializer_list<handle_slot*> const& slots) { m_handles.add(slots); }
	~handle() { clear(); }

	handle& operator=(handle_slot& slot) {
		clear();
		m_handles.add(&slot);
		return *this;
	}

	handle& operator+=(handle_slot& slot) {
		m_handles.add(&slot);
		return *this;
	}

	operator bool() const noexcept {
		return !m_handles.empty();
	}

	void clear() {
		while(!m_handles.empty())
			m_handles.begin()->force_remove();
	}
};

} // namespace stx

#endif // headguard STX_HANDLE_HPP_INCLUDED

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
