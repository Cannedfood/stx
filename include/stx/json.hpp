#pragma once

#include <string_view>

namespace stx::json {

class node;

enum class node_type {
	null,
	number,
	string,
	object,
	array,
	object_key
};

class iterator {
	node* m_current;
public:
	constexpr iterator(node* n = nullptr) noexcept : m_current(n) {}

	constexpr iterator& operator++(void) noexcept;
	constexpr iterator  operator++(int)  noexcept;

	constexpr bool operator==(iterator const& other) const noexcept;
	constexpr bool operator!=(iterator const& other) const noexcept;

	constexpr node& operator*()  noexcept { return *m_current; }
	constexpr node* operator->() noexcept { return m_current; }
};

class node {
	node_type m_type;

	node* m_next     = nullptr;
	node* m_children = nullptr;

	std::string_view m_value;
public:
	constexpr node() noexcept
	: m_type(node_type::null), m_next(nullptr), m_children(nullptr)
	{}

	using iterator = ::stx::json::iterator;

	constexpr void insert(node* n) noexcept {
		n->m_next = m_next;
		m_next = n;
	}

	constexpr node_type        type() const noexcept { return m_type; }
	constexpr std::string_view value() const noexcept { return m_value; }
	constexpr node*            next() const noexcept { return m_next; }
	constexpr node*            children() const noexcept { return m_children; }

	iterator begin() { return {children()}; }
	iterator end()   { return {nullptr}; }

	node* operator[](std::string_view key) noexcept {
		for(node& n : *this) {
			if(n.value() == key)
				return &n;
		}
		return nullptr;
	}

	node* operator[](size_t index) noexcept {
		size_t idx = 0;
		for(node& n : *this) {
			if(idx == index) return &n;
			++idx;
		}
		return nullptr;
	}
};

} // namespace stx::json

// =============================================================
// == Inline Implementation =============================================
// =============================================================

namespace stx::json {

// ** iterator *******************************************************

constexpr iterator& iterator::operator++(void) noexcept { m_current = m_current->next(); return *this; }
constexpr iterator  iterator::operator++(int)  noexcept { return iterator(m_current->next()); }

constexpr bool iterator::operator==(iterator const& other) const noexcept { return m_current == other.m_current; }
constexpr bool iterator::operator!=(iterator const& other) const noexcept { return m_current != other.m_current; }

} // namespace stx::json
