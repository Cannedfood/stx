// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

// TODO: xml::node::m_name also contains the content of comments etc. how do we make this clear to library users?

#ifndef STX_XML_HPP_INCLUDED
#define STX_XML_HPP_INCLUDED

#pragma once

#include "dlist.hpp"
#include "allocator.hpp"

#include <exception>
#include <string_view>
#include <string>

#include <iosfwd>

namespace stx {
namespace xml {

class attribute;
class node;

class node_iterator;

constexpr inline
size_t name_hash(std::string_view sv) noexcept;

class attribute : public dlist_element<attribute> {
public:
	std::string_view const& name() const noexcept { return m_name; }

	template<class T = std::string_view>
	T value() const { return m_value; }

	using dlist_element::next;
	using dlist_element::prev;
	attribute* next(std::string_view const& name) noexcept;
	attribute* prev(std::string_view const& name) noexcept;
	attribute& req_next(std::string_view const& name);
	attribute& req_prev(std::string_view const& name);

	const char* parse(arena_allocator& alloc, const char* s);
private:
	std::string_view m_name;
	std::string_view m_value;
};

class node : public dlist_element<node> {
public:
	enum node_type {
		unassigned_node,
		doctype_node,
		cdata_node, // TODO: should those be regular content nodes? (Annoying for serialization)
		regular_node,
		content_node,
		comment_node
	};

	node_type               type() const noexcept { return m_type; }
	std::string_view const& name() const noexcept { return m_name; }
	std::string_view const& cdata_value() const noexcept { return m_name; }
	std::string_view const& comment_value() const noexcept { return m_name; }
	std::string_view const& content_value() const noexcept { return m_name; }

	node* parent() const noexcept { return m_parent; }

	// Sibling accessors
	using dlist_element::next;
	using dlist_element::prev;

	node* next(std::string_view const& name) noexcept;
	node* prev(std::string_view const& name) noexcept;
	node& req_next(std::string_view const& name);
	node& req_prev(std::string_view const& name);

	node* next(node_type type) noexcept;
	node* prev(node_type type) noexcept;
	node& req_next(node_type type);
	node& req_prev(node_type type);

	// Child accessors
	node* children() const noexcept { return m_children; }

	node* child(std::string_view const& name) noexcept;
	node* child(node_type type) noexcept;

	node& req_child(std::string_view const& name);
	node& req_child(node_type type);

	// Attribute accessors
	attribute* attributes() const noexcept { return m_attributes; }
	attribute* attrib(std::string_view const& name) noexcept;
	attribute& req_attrib(std::string_view const& name);
	template<class T>
	T attrib(std::string_view const& name, T alternative) noexcept;
	template<class T>
	T req_attrib(std::string_view const& name);

	// Iterator
	using iterator = node_iterator;
	iterator begin();
	iterator end();

	// Name hash for switch(node) { case stx::xml::name_hash("thing"): break; }
	constexpr inline operator size_t() const noexcept { return name_hash(m_name); }

	// Comparisons
	constexpr inline bool operator==(std::string_view const& other) const noexcept { return m_name == other; }
	constexpr inline bool operator!=(std::string_view const& other) const noexcept { return m_name != other; }

	// ostream
	void print(std::ostream& stream, unsigned indent = 0);

	// Parsing
	const char* parse_document(arena_allocator&, const char* cstr);
	const char* parse_regular(arena_allocator&, const char*);
	const char* parse_doctype(arena_allocator&, const char*);
	const char* parse_comment(arena_allocator&, const char*);
	const char* parse_content(arena_allocator&, const char*);
	const char* parse_attributes(arena_allocator&, const char*);
	const char* parse_children(arena_allocator&, const char*);
	const char* parse_node(arena_allocator&, const char*);
private:
	node_type        m_type = unassigned_node;
	std::string_view m_name;
	attribute*       m_attributes = nullptr;
	node*            m_parent     = nullptr;
	node*            m_children   = nullptr;
};

std::string load_document(std::string_view path);

struct cursor_location {
	unsigned         line;
	unsigned         column;
	std::string_view line_content;

	cursor_location(unsigned line, unsigned column);
	cursor_location(const char* start, const char* location);
};

namespace errors {
	class error : public std::exception {
		std::string m_message;
		const char* m_location;
		std::string_view m_location2;
	public:
		error(std::string message, const char* location = nullptr, std::string_view location2 = {}) :
			m_message(std::move(message)),
			m_location(location), m_location2(location2)
		{}

		const char* what() const noexcept override { return m_message.c_str(); }
		const char* location() const noexcept { return m_location; }
		std::string_view location2() const noexcept { return m_location2; }

		void diagnose(
			const char* sourcefile,
			const char* source,
			std::ostream& printTo);
		void diagnose(
			const char* sourcefile,
			const char* source);
	};

	class parsing_error : public error {
	public: using error::error;
	};

	class attribute_not_found : public error {
	public: using error::error;
	};

	class node_not_found : public error {
	public: using error::error;
	};
} // namespace errors

} // namespace xml
} // namespace stx

// =============================================================
// == Inline implementation =============================================
// =============================================================

namespace stx {
namespace xml {

constexpr inline
size_t name_hash(std::string_view sv) noexcept {
	// FNV 1a hash
	if constexpr(sizeof(size_t) == 4) {
		size_t result = 2166136261ul;
		for(auto& c : sv) {
			result ^= c;
			result *= 16777619ul;
		}
		return result;
	}
	else if constexpr(sizeof(size_t) == 8) {
		size_t result = 14695981039346656037ul;
		for(auto& c : sv) {
			result ^= c;
			result *= 1099511628211ul;
		}
		return result;
	}
}

// ** attribute *******************************************************

template<>
bool attribute::value<bool>() const;
template<>
int attribute::value<int>() const;
template<>
float attribute::value<float>() const;
template<>
double attribute::value<double>() const;

// ** node *******************************************************

template<class T>
T node::attrib(std::string_view const& name, T alternative) noexcept {
	auto* atb = attrib(name);
	return atb ? atb->value<T>() : alternative;
}
template<class T>
T node::req_attrib(std::string_view const& name) {
	return req_attrib(name).value<T>();
}

class node_iterator {
	node* m_current;
public:
	node_iterator(node* n = nullptr) : m_current(n) {}

	node& operator*() noexcept { return *m_current; }
	node* operator->() noexcept { return m_current; }
	node const& operator*() const noexcept { return *m_current; }
	node const* operator->() const noexcept { return m_current; }
	node_iterator& operator++() { m_current = m_current->next(); return *this; }
	node_iterator  operator++(int) { auto copy = *this; return ++copy; }
	operator bool() const noexcept { return m_current != nullptr; }
};

inline node_iterator node::begin() { return node_iterator(children()); }
inline node_iterator node::end() { return node_iterator(); }

} // namespace xml
} // namespace stx

#endif // header guard STX_XML_HPP_INCLUDED

/*
 Copyright (c) 2018 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
