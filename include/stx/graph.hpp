#pragma once

#include <type_traits>
#include <utility>

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

	void update_next_last() noexcept;

protected:
	constexpr
	list_element();
	~list_element() noexcept;

	// -- Move --------------------------------------------------------------
	constexpr
	list_element(Tself&& e);
	constexpr
	Tself& operator=(Tself&& e) noexcept;

	// -- Copy --------------------------------------------------------------
	list_element(Tself const&)     = delete;
	Tself& operator=(Tself const&) = delete;

public:

	// -- Functionality -----------------------------------------------------
	void remove() noexcept;

	void push_front(T* p) noexcept;
	void push_back(T* p) noexcept;

	void insert_as_last(T* p) noexcept;

	void insert_as_next(T* p) noexcept;

	// -- Getters / Setters -------------------------------------------------
	      T* next()       noexcept { return m_next; }
	      T* last()       noexcept { return m_last; }
	const T* next() const noexcept { return m_next; }
	const T* last() const noexcept { return m_last; }

	      T* tail()       noexcept;
	const T* tail() const noexcept;

	      T* head()       noexcept;
	const T* head() const noexcept;
};

template<typename T, typename TParent>
class child_element;

template<typename T, typename TChild>
class parent_element {
	using Tself  = parent_element<T, TChild>;

	TChild* m_children;

	friend T;
	friend class child_element<TChild, T>;

protected:
	using parent_element_t = parent_element<T, TChild>;
	using child_element_t  = child_element<TChild, T>;

	parent_element();
	~parent_element() noexcept;

public:
	// -- Move --------------------------------------------------------------
	parent_element(Tself&& e);
	Tself& operator=(Tself&& other) noexcept;

	// -- Copy --------------------------------------------------------------
	parent_element(Tself const&) = delete;
	Tself& operator=(Tself const&) = delete;

	// -- Functionality -----------------------------------------------------
	void push_back(TChild* c) noexcept;
	void push_front(TChild* c) noexcept;

	// -- Getters / Setters -------------------------------------------------
	      TChild* children()       noexcept { return m_children; }
	const TChild* children() const noexcept { return m_children; }
};

template<typename T, typename TParent>
class child_element : public list_element<T> {
	mutable TParent* m_parent;

	friend class parent_element<TParent, T>;
protected:
	using parent_element_t = parent_element<TParent, T>;
	using child_element_t  = child_element<T, TParent>;

	constexpr
	child_element();
	~child_element() noexcept;

public:
	// -- Move --------------------------------------------------------------
	constexpr
	child_element(child_element<T, TParent>&& other);
	constexpr
	child_element<T, TParent>& operator=(child_element<T, TParent>&& other) noexcept;

	// -- Functionality -----------------------------------------------------
	void push_front(T* t) noexcept;
	void push_back(T* t) noexcept;

	void remove() noexcept;

	// -- Getters / Setters -------------------------------------------------
	TParent* parent() const noexcept { return m_parent; }
};

#ifdef STX_WIP

template<typename T>
class tree_node :
	public parent_element<T, T>,
	public child_element <T, T>
{
	// should need nothing but constructor & destructor
};

#endif // STX_WIP

} // namespace stx

#include "graph.inl"
