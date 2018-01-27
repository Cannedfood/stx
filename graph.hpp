#ifndef STX_GRAPH_HPP_INCLUDED
#define STX_GRAPH_HPP_INCLUDED

#pragma once

#include <initializer_list>

namespace stx {

template<class T>
struct list;
template<class T>
struct list_element;

namespace detail {
	template<class T> class list_element_iterator;
	template<class T> class tree_element_iterator;
} // namespace detail

template<class T>
struct list_element {
	using list_element_t = list_element<T>;
	using list_t         = list<T>;

	using iterator       = detail::list_element_iterator<T>;
	using const_iterator = const iterator;

	constexpr list_element() noexcept;
	constexpr list_element(list_element&& l) noexcept;

	~list_element() noexcept;

	constexpr list_element_t& operator=(list_element_t&& t) noexcept;
	constexpr list_element_t& operator=(list_element_t const& t) noexcept;

	constexpr bool remove() noexcept;
	constexpr void insert(T* new_next) noexcept;
	constexpr void insert_to(T*& t) noexcept;

	constexpr T*  const& next()    const noexcept { return m_next; }
	constexpr T** const& to_this() const noexcept { return m_to_this; }

	constexpr T* back(T* end = nullptr) noexcept;

	constexpr iterator begin() noexcept;
	constexpr iterator end() noexcept;
	constexpr const_iterator cbegin() const noexcept;
	constexpr const_iterator cend() const noexcept;
private:
	T*  m_next;
	T** m_to_this;

	T*       _this()       noexcept { return static_cast<T*>(this); }
	T const* _this() const noexcept { return static_cast<T const*>(this); }
	constexpr list_element_t* _next() noexcept { return m_next; }
	constexpr list_element_t const* _next() const noexcept { return m_next; }

	constexpr void _reset(T** toThis, T* next) noexcept;

	friend list_t;
	friend iterator;
};

template<class T>
struct list {
	using list_t         = list<T>;
	using list_element_t = list_element<T>;

	using iterator = typename list_element_t::iterator;
	using const_iterator = typename list_element_t::const_iterator;

	constexpr list() noexcept;
	constexpr list(list_t&& l) noexcept;
	constexpr list_t& operator=(list_t&& l);
	~list() noexcept;

	constexpr void add(T* t) noexcept;
	constexpr void add(std::initializer_list<T*> t) noexcept;
	constexpr void clear() noexcept;

	constexpr T* back(T* end = nullptr) noexcept;

	constexpr iterator begin() noexcept;
	constexpr iterator end() noexcept;
	constexpr const_iterator cbegin() const noexcept;
	constexpr const_iterator cend() const noexcept;
private:
	T* m_elements;
};

template<class T>
struct tree_element : public list_element<T>, public list<T> {
	using tree_element_t = tree_element<T>;
};

// =============================================================
// == implementation =============================================
// =============================================================

// ** detail::list_element_iterator<T> *************************************
namespace detail {

template<class T>
class list_element_iterator {
	mutable list_element<T>* m_element;
public:
	list_element_iterator() : m_element(nullptr) {}
	list_element_iterator(list_element<T>* t) : m_element(t) {}

	constexpr
	bool operator==(list_element_iterator<T> const& t) const noexcept {
		return m_element == t.m_element;
	}

	T* operator->() noexcept { return m_element->_this(); }
	T& operator*() noexcept { return *m_element->_this(); }
	T const* operator->() const noexcept { return m_element->_this(); }
	T const& operator*() const noexcept { return *m_element->_this(); }

	operator bool() const noexcept { return m_element; }

	list_element_iterator<T> const& operator++() const noexcept {
		m_element = m_element->_next();
		return *this;
	}
	list_element_iterator<T>& operator++() noexcept {
		m_element = m_element->_next();
		return *this;
	}
	list_element_iterator<T> operator++(int) noexcept {
		return ++list_element_iterator<T>();
	}
	const list_element_iterator<T> operator++(int) const noexcept {
		return ++list_element_iterator<T>();
	}
};

} // namespace detail


// ** list_element<T> *******************************************************

template<class T> constexpr
list_element<T>::list_element() noexcept :
	m_next(nullptr),
	m_to_this(nullptr)
{}

template<class T> constexpr
list_element<T>::list_element(list_element&& l) noexcept :
	list_element()
{
	*this = (list_element&&)(l);
}

template<class T>
list_element<T>::~list_element() noexcept { remove(); }

template<class T> constexpr
list_element<T>& list_element<T>::operator=(list_element_t&& t) noexcept {
	_reset(t.m_to_this, t.m_next);
	t.m_next = nullptr;
	t.m_to_this = nullptr;
	return *this;
}

template<class T> constexpr
list_element<T>& list_element<T>::operator=(list_element_t const& t) noexcept {
	// Do nothing
	return *this;
}

template<class T> constexpr
bool list_element<T>::remove() noexcept {
	if(m_to_this) {
		*m_to_this = m_next;
		if(m_next) {
			_next()->m_to_this = m_to_this;
			m_next = nullptr;
		}
		m_to_this = nullptr;
		return true;
	}
	else if(m_next) {
		_next()->m_to_this = m_to_this;
		m_next = nullptr;
		return true;
	}
	return false;
}

template<class T> constexpr
void list_element<T>::insert(T* new_next) noexcept {
	new_next->_reset(&m_next, m_next);
}

template<class T> constexpr
void list_element<T>::insert_to(T*& t) noexcept {
	_reset(&t, t);
}

template<class T> constexpr
T* list_element<T>::back(T* end) noexcept {
	T* t = this;
	while((list_element<T>*)(t)->next() != end) {
		t = (list_element<T>*)(t)->next();
	}
	return t;
}

template<class T> constexpr
typename list_element<T>::iterator list_element<T>::begin() noexcept { return iterator(this); }
template<class T> constexpr
typename list_element<T>::iterator list_element<T>::end() noexcept { return iterator(); }
template<class T> constexpr
typename list_element<T>::const_iterator list_element<T>::cbegin() const noexcept { return const_iterator(this); }
template<class T> constexpr
typename list_element<T>::const_iterator list_element<T>::cend() const noexcept { return const_iterator(); }

template<class T> constexpr
void list_element<T>::_reset(T** toThis, T* next) noexcept {
	remove();

	if((m_to_this = toThis))
		*m_to_this = _this();

	if((m_next = next))
		((list_element_t*)m_next)->m_to_this = &m_next;
}

// ** list<T> *******************************************************

template<class T> constexpr
list<T>::list() noexcept :
	m_elements(nullptr)
{}

template<class T> constexpr
list<T>::list(list_t&& l) noexcept :
	list()
{
	*this = (list_t&&)(l);
}

template<class T> constexpr
list<T>& list<T>::operator=(list_t&& l) {
	clear();
	if((m_elements = l.m_elements)) {
		l.m_elements = nullptr;
		m_elements->m_to_this = &m_elements;
	}
	return *this;
}

template<class T>
list<T>::~list() noexcept {
	clear();
}

template<class T> constexpr
void list<T>::add(T* t) noexcept {
	((list_element_t*)t)->insert_to(m_elements);
}

template<class T> constexpr
void list<T>::add(std::initializer_list<T*> t) noexcept {
	for(auto* tt : t)
		add(tt);
}

template<class T> constexpr
void list<T>::clear() noexcept {
	while(m_elements)
		((list_element_t*)m_elements)->remove();
}

template<class T> constexpr
typename list<T>::iterator list<T>::begin() noexcept { return iterator(m_elements); }
template<class T> constexpr
typename list<T>::iterator list<T>::end() noexcept { return iterator(); }
template<class T> constexpr
typename list<T>::const_iterator list<T>::cbegin() const noexcept { return const_iterator(m_elements); }
template<class T> constexpr
typename list<T>::const_iterator list<T>::cend() const noexcept { return const_iterator(); }

} // namespace stx

#endif // header guard STX_GRAPH_HPP_INCLUDED
