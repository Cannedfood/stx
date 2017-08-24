#pragma once

#include <type_traits>
#include <utility>

#ifdef STX_DEBUG
#	include "../assert.hpp"
#endif

namespace stx {

template<typename T>
class list_element {
	T* m_next;
	T* m_last;
public:


	T* const next() const noexcept { return m_next; }
	T*       next()       noexcept { return m_next; }
	T* const last() const noexcept { return m_next; }
	T*       last()       noexcept { return m_next; }
	T*       head() noexcept {
		T* t;
		for(t = static_cast<T*>(this); t->m_last; t = t->m_last);
		return t;
	}
};

} // namespace stx
