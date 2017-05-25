#pragma once

#include <type_traits>

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

} // namespace stx
