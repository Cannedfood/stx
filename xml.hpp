// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#pragma once

#include "dlist.hpp"
#include "allocator.hpp"

#include <exception>
#include <string_view>

namespace stx {
namespace xml {

class parser;
class attribute;
class node;

class attribute : public dlist<attribute> {
public:
	std::string_view const& name() const noexcept { return m_name; }
	std::string_view const& value() const noexcept { return m_value; }

	using dlist::next;
	using dlist::prev;
	attribute* next(std::string_view const& name) noexcept;
	attribute* prev(std::string_view const& name) noexcept;

	const char* parse(arena_allocator* alloc, const char* s);
private:
	std::string_view m_name;
	std::string_view m_value;
};

class node : public dlist<node> {
public:
	enum node_type {
		unassigned_node,
		doctype_node,
		cdata_node,
		regular_node,
		content_node,
		comment_node
	};

	node_type               type()       const noexcept { return m_type; }
	std::string_view const& name()       const noexcept { return m_name; }

	node* parent() const noexcept { return m_parent; }
	node* children() const noexcept { return m_children; }

	using dlist::next;
	using dlist::prev;
	node* next(std::string_view const& name) noexcept;
	node* prev(std::string_view const& name) noexcept;
	node* child(std::string_view const& name) noexcept;

	attribute* attributes() const noexcept { return m_attributes; }
	attribute* attrib(std::string_view const& name) noexcept;

	const char* parse_regular(arena_allocator*, const char*);
	const char* parse_doctype(arena_allocator*, const char*);
	const char* parse_comment(arena_allocator*, const char*);
	const char* parse_content(arena_allocator*, const char*);
	const char* parse_attributes(arena_allocator*, const char*);
	const char* parse_children(arena_allocator*, const char*);
	const char* parse(arena_allocator* alloc, const char* s);
private:
	node_type        m_type = unassigned_node;
	std::string_view m_name;
	attribute*       m_attributes = nullptr;
	node*            m_parent     = nullptr;
	node*            m_children   = nullptr;
};

class parser : public arena_allocator {
public:
	parser();

	node* parse(const char* cstr);
	node* load(const char* path);
};

struct cursor_location {
	unsigned line;
	unsigned column;

	cursor_location(unsigned line, unsigned column);
	cursor_location(const char* start, const char* location);
};

class error : public std::exception {
	const char* m_message;
	const char* m_location;
	std::string_view m_location2;
public:
	error(const char* message, const char* location = nullptr, std::string_view location2 = {}) :
		m_message(message), m_location(location), m_location2(location2)
	{}

	const char* what() const noexcept override { return m_message; }
	const char* location() const noexcept { return m_location; }
	std::string_view location2() const noexcept { return m_location2; }
};

} // namespace xml
} // namespace stx

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
