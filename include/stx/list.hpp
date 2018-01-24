// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#ifndef STX_GRAPH_HPP_INCLUDED
#define STX_GRAPH_HPP_INCLUDED

#pragma once

#include <type_traits>

namespace stx {

template<class T>
class list_element {
	T*  m_next;
	T** m_to_this;
public:
	class iterator;
	using const_iterator = const iterator;
	using list_element_t = list_element<T>;

	constexpr list_element();
	constexpr list_element(list_element_t const& other);
	constexpr list_element(list_element_t&& other);
	constexpr list_element_t& operator=(list_element_t const& other);
	constexpr list_element_t& operator=(list_element_t&& other);

	T*       const& next()          { return m_next; }
	T const* const& next()    const { return m_next; }
	T**             to_this() const { return m_to_this; }

	constexpr bool remove();
	constexpr void add_to(T*& t);
	constexpr void add_to(list_element<T>& t);

	constexpr T* snip();

	constexpr T* back(T* end = nullptr);
	constexpr T* center(T* end = nullptr);

	iterator       begin()      { return iterator(this); }
	const_iterator cbegin()     { return iterator(this); }
	iterator       end()  const { return iterator(); }
	const_iterator cend() const { return iterator(); }
};

} // namespace stx

#include "list.inl"

#endif // STX_GRAPH_HPP_INCLUDED

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
