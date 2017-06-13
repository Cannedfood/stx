#pragma once

#include <type_traits>

#ifdef STX_DEBUG
#	include "assert.hpp"
#endif

namespace stx {

template<typename T>
class list_element {
public:
	using Tself = list_element<T>;
private:
	T* m_next;
	T* m_last;

	void update_next_last() noexcept {
		if(m_next)
			m_next->m_last = this;
		if(m_last)
			m_last->m_next = this;
	}

protected:
	void next(T* t) noexcept {
		if(m_next)
			m_next->m_last = nullptr;
		if((m_next = t))
			m_next->m_last = this;
	}
	void last(T* t) noexcept {
		if(m_last)
			m_last->m_next = nullptr;
		if((m_last = t))
			m_last->m_next = this;
	}

	constexpr inline
	list_element() :
		m_next(nullptr),
		m_last(nullptr)
	{}

	constexpr
	list_element(Tself&& e) :
		m_next(e.m_next),
		m_last(e.m_last)
	{
		e.m_next = e.m_last = nullptr;
		update_next_last();
	}

	~list_element() noexcept {
		remove();
	}

	constexpr
	Tself& operator=(Tself&& e) noexcept {
		remove();
		m_next = e.m_next;
		m_last = e.m_last;
		e.m_next = e.m_last = nullptr;
		update_next_last();
	}

	list_element(Tself const&)     = delete;
	Tself& operator=(Tself const&) = delete;

public:
	void remove() noexcept {
		if(m_next) {
			m_next->m_last = m_last;
		}
		if(m_last) {
			m_last->m_next = m_next;
		}
		m_next = nullptr;
		m_last = nullptr;
	}

	      T* next()       noexcept { return m_next; }
	      T* last()       noexcept { return m_last; }
	const T* next() const noexcept { return m_next; }
	const T* last() const noexcept { return m_last; }
};

template<typename T>
class pointer_pair {
	using Tself  = pointer_pair<T>;
	using Tother = T;

	T* m_other;

public:
	pointer_pair(Tself const&) = delete;

	pointer_pair() : m_other(nullptr) {}
	pointer_pair(Tself&& other) :
		pointer_pair()
	{
		*this = std::move(other);
	}

	~pointer_pair() noexcept {
		reset();
	}

	Tself& operator=(Tself const&) = delete;
	Tself& operator=(Tself&& other) noexcept {
		reset(other.m_other);
		return *this;
	}

	void reset(T* other = nullptr) noexcept {
		if(m_other) {
#ifdef STX_DEBUG
			xassert(m_other->m_other == this);
#endif
			m_other->m_other = nullptr;
		}

		m_other = other;
		if(m_other) {
			m_other->reset();
			m_other->m_other = (decltype(m_other->m_other)) this;
		}
	}

	T* get() noexcept { return m_other; }
};

#ifdef STX_WIP

template<typename T, typename TParent>
class child_element;

template<typename T, typename TChild>
class parent_element {
	using Tself  = parent_element<T, TChild>;

	TChild* m_children;

	friend T;
	friend class child_element<TChild, parent_element<T, TChild>>;
protected:
	~parent_element();

public:
	TChild*       children()       { return m_children; }
	TChild const* children() const { return m_children; }

	parent_element() :
		m_children()
	{}

	parent_element(Tself const&) = delete;

	parent_element(Tself&& e) :
		m_children(e.m_children)
	{
		e.m_children = nullptr;
	}

	Tself& operator=(Tself const&) = delete;
	Tself& operator=(Tself&& other) {
		std::swap(m_children, other.m_children);
	}
};

template<typename T, typename TParent>
class child_element : public list_element<T> {
	using parent_type = TParent;

	static_assert(std::is_base_of<parent_element<TParent, T>, TParent>::value, "TParent must inherit parent_element<T>");

	parent_type* m_parent;

	friend T;
	friend TParent;
	friend class parent_element<T, child_element<T, TParent>>;
protected:
	child_element() {}
	~child_element() {
		// remove();
	}

public:
	parent_type parent() { return m_parent; }
};

template<typename T>
class tree_node :
	public parent_element<T, T>,
	public child_element <T, T>
{
	// should need nothing but constructor & destructor
};

#endif // STX_WIP

} // namespace stx
